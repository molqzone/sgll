#!/usr/bin/env python3
"""Audit sgll device definitions and reset mapping against the TRM and SDK.

Inputs (override the default locations with command-line options):
  - TRM v1.02 文本提取 / text extraction:
    BSP docs/reference/sg2002_trm_en_v1.02.txt
    (pymupdf extraction of the English TRM; page markers included).
  - Pinned SG200x SDK worktree (hal/cv181x/config/intr_conf.h).
  - sgll/inc headers, src/sg200x_ll_rcc.c, and driver/sg200x_clock_tree.hpp.

Run:  python3 sgll/tools/audit_vs_trm.py   (exit 0 = consistent)

Checks:
  1. clk_en gate bits   (TRM Tables 8.52-8.56 vs GATE_* macros)
  2. bypass bits        (Tables 8.58/8.59 vs BYPASS_* macros)
  3. soft reset bits    (Tables 7.2-7.5 vs device reset location table)
  4. divider offsets    (Table 8.51 vs CLKGEN_DIV_OFF_* constants)
  5. factor widths/init (driver clock tree vs named div leaves)
  6. memory map         (Table 3.4 vs *_BASE constants)
  7. C906L IRQ numbers  (SDK intr_conf.h vs IRQ_* constants)
"""
import argparse
import os
import re
import sys
from pathlib import Path

SGLL_ROOT = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--bsp-root', type=Path,
                    default=Path(os.environ.get('SG200X_BSP_ROOT', SGLL_ROOT.parent)),
                    help='BSP containing the clock tree and reference documents')
parser.add_argument('--sdk-root', type=Path,
                    help='SDK checkout or its freertos/cvitek directory')
parser.add_argument('--trm', type=Path, help='Extracted SG2002 TRM v1.02 text')
parser.add_argument('--clock-tree', type=Path, help='C++ clock-tree header to audit')
args = parser.parse_args()
BSP_ROOT = args.bsp_root.resolve()
SDK = args.sdk_root or Path(os.environ.get('SG200X_SDK_DIR', BSP_ROOT / 'build/sdk-worktree'))
if (SDK / 'freertos/cvitek').is_dir():
    SDK = SDK / 'freertos/cvitek'
SGLL = SGLL_ROOT / 'inc'
DRIVER = args.clock_tree or BSP_ROOT / 'driver/sg200x_clock_tree.hpp'
trm_path = args.trm or BSP_ROOT / 'docs/reference/sg2002_trm_en_v1.02.txt'
for label, path, option in (
        ('TRM text', trm_path, '--trm'),
        ('clock tree', DRIVER, '--clock-tree'),
        ('SDK interrupt map', SDK / 'hal/cv181x/config/intr_conf.h', '--sdk-root')):
    if not path.is_file():
        parser.error(f'Missing {label}: {path}; specify {option}')
TRM = trm_path.read_text(encoding='utf-8').split('\n')

issues = []

def blob(start_pat, end_pat):
    s = e = None
    for i, l in enumerate(TRM):
        if re.search(start_pat, l):
            s = i
        elif s is not None and re.search(end_pat, l) and i > s:
            e = i
            break
    if s is None:
        return ''
    text = ' '.join(x.strip() for x in TRM[s:e])
    text = re.sub(r'===== PAGE \d+ =====|Copyright © 2025 SOPHGO Co\., Ltd \d*|'
                  r'continues on next page|Table [0-9.]+ – continued from previous page|'
                  r'CHAPTER \d+\. [A-Z ]+', ' ', text)
    return re.sub(r'\s+', ' ', text)

def parse_bit_table(table_pat, next_pat, bit_name_pat, desc_pat):
    out = {}
    b = blob(table_pat, next_pat)
    for m in re.finditer(r'(?<!\d)((?:3[01]|[12]?\d))(?:(?<![:\d])(?:\s*:)\s*(3[01]|[12]?\d))?(?!\d)\s+'
                         + bit_name_pat + r'\s+(R/W|RO|RWS)\s+(.{0,120}?)\s+0x[0-9A-Fa-f]+', b):
        lo = int(m.group(1))
        hi = int(m.group(2)) if m.group(2) else lo
        if hi != lo:
            continue
        desc = m.group(4)
        dm = re.search(desc_pat, desc)
        if dm:
            out[lo] = re.sub(r'\(.*$', '', (dm.group(1) if dm.groups() else desc)).strip().rstrip('.')
    return out

# ---------------------------------------------------------------- TRM side
trm_gates = {}
for n in range(5):
    tab = parse_bit_table(rf'Table 8\.5{2 + n}: clk_en_{n}',
                          rf'Table 8\.5{3 + n}:', r'(?:clk_en_%d_\d+|Res\w+)' % n,
                          r'Clock Enable for (\S+)')
    for bit, name in tab.items():
        trm_gates[(n, bit)] = name

trm_byp0 = parse_bit_table(r'Table 8\.58: clk_byp_0', r'Table 8\.59:',
                           r'(?:clk_byp_0_\d+|Res\w+)', r'for (?:.* )?clk_(\S+)')
trm_byp1 = parse_bit_table(r'Table 8\.59: clk_byp_1', r'Table 8\.6[0-9]:',
                           r'(?:clk_byp_1_\d+|Res\w+)', r'for (?:.* )?clk_(\S+)')

trm_rst = {}
for n, nxt in [(0, 1), (1, 2), (2, 3), (3, 4)]:
    b = blob(rf'Table 7\.{2 + n}: SOFT_RSTN_{n}', rf'Table 7\.{2 + nxt}:')
    for m in re.finditer(r'(?<!\d)(3[01]|[12]?\d)(?!\d)\s+reg\s*_?soft\s*_?reset\s*_?x\s*_?(\w+)\s+R/W', b):
        trm_rst[(n, int(m.group(1)))] = m.group(2)

trm_div_off = {}
b = blob(r'Table 8\.51: Clock Gen', r'8\.9 IP/Subsystem')
for m in re.finditer(r'div_clk_([a-z0-9_]+?)\s+0x([0-9a-fA-F]+)\s+divider', b):
    trm_div_off[m.group(1)] = int(m.group(2), 16)

# ---------------------------------------------------------------- sgll side
device = open(f'{SGLL}/sg2002.h').read()
rcc_source = open(f'{SGLL}/../src/sg200x_ll_rcc.c').read()
driver = open(DRIVER).read()

def canon_clk(name):
    n = re.sub(r'^clk_', '', name.lower()).upper()
    return n.replace('RTCSYS_SRC_0', 'RTCSYS_SRC0')

def expand_clkgen_reg(token):
    token = token.strip()
    direct = re.fullmatch(r'(\d+)U?', token)
    if direct:
        return int(direct.group(1))
    m = re.fullmatch(r'CLKGEN_CLK_EN([0-4])_OFFSET', token)
    if m:
        return int(m.group(1))
    m = re.fullmatch(r'CLKGEN_CLK_BYP([01])_OFFSET', token)
    if m:
        return int(m.group(1))
    return integer_token(token) if 'integer_token' in globals() else None

sgll_gates = {}
for m in re.finditer(r'#define CLKGEN_GATE_(\w+)\s+([^,\s]+),\s*(\d+)U?', device):
    reg = expand_clkgen_reg(m.group(2))
    if reg is not None:
        sgll_gates[(reg, int(m.group(3)))] = m.group(1)

sgll_byp = {}
for m in re.finditer(r'#define CLKGEN_BYPASS_(\w+)\s+([^,\s]+),\s*(\d+)U?', device):
    reg = expand_clkgen_reg(m.group(2))
    if reg is not None:
        sgll_byp[(reg, int(m.group(3)))] = m.group(1)

sgll_rst = {}
if 'SG2002_RESET_MAP(RESET_LOCATION)' not in rcc_source:
    raise RuntimeError('RCC reset table must use the SG2002 device map')
for m in re.finditer(r'X\(\s*(\w+),\s*(0x[0-9A-Fa-f]+|\d+)U?,\s*(0x[0-9A-Fa-f]+|\d+)U?\s*\)', device):
    sgll_rst[(int(m.group(2), 0), int(m.group(3), 0))] = m.group(1)
if len(sgll_rst) != 48:
    raise RuntimeError('SG2002 reset coordinate map is incomplete')

# Device scalars are C23 constexpr declarations; feature flags remain macros.
# These checks need literal values only, not arbitrary C expression evaluation.
device_uints = {}
uint_literal = r"((?:0[xX][0-9A-Fa-f']+|[0-9][0-9']*)(?:[uUlL]+)?)"
for pattern in (
        rf'^static constexpr auto\s+(\w+)\s*=\s*{uint_literal}\s*;',
        rf'^#define\s+(\w+)\s+{uint_literal}\s*(?:/\*.*?\*/)?$'):
    for m in re.finditer(pattern, device, re.M):
        token = re.sub(r'[uUlL]+$', '', m.group(2)).replace("'", '')
        device_uints[m.group(1)] = int(token, 0)

sgll_defs = {name: value for name, value in device_uints.items()
             if name.endswith(('_BASE', '_BASE_ADDRESS'))}
sgll_div_off = {name.removeprefix('CLKGEN_DIV_OFF_'): value
                for name, value in device_uints.items()
                if name.startswith('CLKGEN_DIV_OFF_')}

def integer_token(token):
    token = token.strip()
    if token in device_uints:
        return device_uints[token]
    token = re.sub(r'[uUlL]+$', '', token).replace("'", '')
    return int(token, 0)

LINUX_OWNED_GATE = re.compile(
    r'^(A53|CPU_AXI0|CPU_GIC|XTAL_A53|TPU|AHB_ROM|DDR_AXI_REG|AXI4_EMMC|EMMC|100K_EMMC|'
    r'AXI4_SD0|SD0|100K_SD0|AXI4_SD1|SD1|100K_SD1|SPI_NAND|500M_ETH0|AXI4_ETH0|AHB_SF|'
    r'DSI_ESC|AXI_VIP|SRC_VIP_SYS_0|SRC_VIP_SYS_1|DISP_SRC_VIP|AXI_VIDEO_CODEC|VC_SRC0|'
    r'H264C|H265C|APB_JPEG|APB_H264C|APB_H265C|CAM0|CAM1|CSI_MAC0_VIP|CSI_MAC1_VIP|'
    r'ISP_TOP_VIP|IMG_D_VIP|IMG_V_VIP|SC_TOP_VIP|SC_D_VIP|SC_V1_VIP|SC_V2_VIP|SC_V3_VIP|'
    r'LDC_VIP|BT_VIP|DISP_VIP|DSI_MAC_VIP|LVDS0_VIP|LVDS1_VIP|CSI0_RX_VIP|CSI1_RX_VIP|'
    r'PAD_VI_VIP|SRC_VIP_SYS_2|PAD_VI1_VIP|CFG_REG_VIP|CFG_REG_VC|AP_DEBUG|PAD_VI2_VIP|'
    r'CSI_BE_VIP|VIP_IP0_EN|VIP_IP1_EN|VIP_IP2_EN|VIP_IP3_EN|C906_0_EN|C906_1_EN|'
    r'SRC_VIP_SYS_3_EN|SRC_VIP_SYS_4_EN|IVE_VIP_EN|RAW_VIP_EN|OSDC_VIP_EN|FBC_VIP_EN|'
    r'CAM0_VIP_EN|JPEG|AP_DEBUG)$')
LINUX_OWNED_RST = {'SYSTEM', 'DDR', 'H264C', 'H265C', 'JPEG', 'VIPSYS',
                   'TDMA', 'TPU', 'TPUSYS', 'USB', 'ETH0', 'ETH1', 'NAND',
                   'EMMC', 'SD0', 'AHB_ROM', 'SPIC', 'SE', 'AUDDAC',
                   'AUDDAC_APB', 'AUDADC', 'VCSYS', 'ETHPHY', 'ETHPHY_APB',
                   'AUDSRC'}

print('=== 1. clk_en gate bits: every sgll GATE_* must match TRM exactly ===')
bad = 0
for (reg, bit), got in sorted(sgll_gates.items()):
    name = trm_gates.get((reg, bit))
    if name is None:
        print(f'  FAIL: GATE_{got} @ en_{reg}[{bit}] has no TRM bit')
        issues.append(f'gate not in TRM: {got} en_{reg}[{bit}]')
        bad += 1
    elif canon_clk(name) != got:
        print(f'  FAIL: GATE_{got} @ en_{reg}[{bit}] TRM names it clk_{name}')
        issues.append(f'gate mismatch: {got} en_{reg}[{bit}] {name}')
        bad += 1
excluded = [(reg, bit, name) for (reg, bit), name in trm_gates.items()
            if (reg, bit) not in sgll_gates]
linux_owned = [x for x in excluded if LINUX_OWNED_GATE.match(canon_clk(x[2]))]
other = [x for x in excluded if x not in linux_owned]
print(f'  sgll gate macros: {len(sgll_gates)}, all TRM-matched: {bad == 0}')
print(f'  TRM gate bits not in sgll: {len(excluded)} '
      f'(Linux-owned by design: {len(linux_owned)}, unclassified: {len(other)})')
for reg, bit, name in other:
    print(f'  UNCLASSIFIED en_{reg}[{bit}] clk_{name} - decide ownership')
    issues.append(f'gate unclassified: en_{reg}[{bit}] {name}')

print('=== 2. bypass bits: sgll BYPASS_* vs Tables 8.58/8.59 ===')
trm_byp_all = {}
for bit, name in trm_byp0.items():
    trm_byp_all[(0, bit)] = name
for bit, name in trm_byp1.items():
    trm_byp_all[(1, bit)] = name
for (reg, bit), got in sorted(sgll_byp.items()):
    name = trm_byp_all.get((reg, bit))
    ok = name is not None and canon_clk(name) == got
    print(f'  BYPASS_{got} @ byp_{reg}[{bit}]: {"OK" if ok else "FAIL vs clk_" + str(name)}')
    if not ok:
        issues.append(f'bypass mismatch {got}')
for (reg, bit), name in sorted(trm_byp_all.items()):
    if (reg, bit) not in sgll_byp:
        print(f'  (TRM byp_{reg}[{bit}] clk_{name} not exposed - by design)')

print('=== 3. SOFT_RSTN bits: TRM and documented CV181x SDK coordinates ===')
# cv181x-resets.h declares RST_SD1=17, while TRM Table 7.2 marks it reserved.
# Retain this existing BSP reset mapping; no other undocumented bit is accepted.
sdk_reset_overrides = {(0, 17): 'SD1'}
bad = 0
sdk_matched = 0
for (reg, bit), got in sorted(sgll_rst.items()):
    tok = trm_rst.get((reg, bit))
    if tok is None:
        if sdk_reset_overrides.get((reg, bit)) == got:
            print(f'  SDK: RESET_{got} @ rstn{reg}[{bit}] from cv181x-resets.h (TRM reserved)')
            sdk_matched += 1
            continue
        print(f'  FAIL: RESET_{got} @ rstn{reg}[{bit}] has no TRM or SDK coordinate')
        issues.append(f'reset not documented: {got} rstn{reg}[{bit}]')
        bad += 1
        continue
    base = re.sub(r'\d+$', '', tok.upper())
    idx = re.search(r'(\d+)$', tok)
    canon = base + (idx.group(1) if idx else '')
    if tok.upper() == 'WDT':
        canon = 'WDT0'
    if canon != got:
        print(f'  FAIL: RESET_{got} @ rstn{reg}[{bit}] TRM says {tok}')
        issues.append(f'reset mismatch {got} rstn{reg}[{bit}] {tok}')
        bad += 1
excluded = [(reg, bit, tok) for (reg, bit), tok in trm_rst.items() if (reg, bit) not in sgll_rst]
linux_owned = [x for x in excluded if re.sub(r'\d+$', '', x[2].upper()) in LINUX_OWNED_RST or
                x[2].upper() in LINUX_OWNED_RST]
other = [x for x in excluded if x not in linux_owned]
print(f'  sgll reset targets: {len(sgll_rst)}, TRM: {len(sgll_rst) - sdk_matched - bad}, '
      f'documented SDK overrides: {sdk_matched}, mismatches: {bad}')
print(f'  TRM reset bits not in sgll: {len(excluded)} (Linux-owned: {len(linux_owned)})')
for reg, bit, tok in other:
    print(f'  UNCLASSIFIED rstn{reg}[{bit}] {tok}')
    issues.append(f'reset unclassified rstn{reg}[{bit}] {tok}')

print('=== 4. div offsets: sgll DIV_OFF_* vs Table 8.51 (case-insensitive) ===')
trm_by_off = {off: name for name, off in trm_div_off.items()}
bad = 0
for name, off in sgll_div_off.items():
    trm_name = trm_by_off.get(off)
    if trm_name is None:
        print(f'  FAIL: DIV_OFF_{name} 0x{off:03X} not in Table 8.51')
        issues.append(f'div offset unknown {name}')
        bad += 1
    else:
        canon = trm_name.upper().replace('PWM_SRC_0', 'PWM_SRC')
        if canon != name:
            print(f'  FAIL: DIV_OFF_{name} 0x{off:03X} TRM calls it div_clk_{trm_name}')
            issues.append(f'div offset name {name} vs {trm_name}')
            bad += 1
print(f'  sgll div offsets: {len(sgll_div_off)}, matched: {len(sgll_div_off) - bad}')
for name, off in sorted(trm_div_off.items(), key=lambda kv: kv[1]):
    canon = name.upper().replace('PWM_SRC_0', 'PWM_SRC')
    if canon not in sgll_div_off:
        print(f'  (TRM div_clk_{name} 0x{off:03X} not exposed - by design)')

print('=== 5. factor widths and initial values vs driver clock tree ===')
driver_div = {}
for m in re.finditer(r'Divide\s*\(([^()]*)\)', driver, re.S):
    args = [arg.strip() for arg in m.group(1).split(',')]
    if len(args) != 4:
        continue
    try:
        driver_div[integer_token(args[0])] = (integer_token(args[2]), integer_token(args[3]))
    except ValueError:
        continue
leaf_off = {'AXI4': '0x0b8', 'AXI6': '0x0bc', '1M': '0x0fc', 'SPI': '0x100',
            'I2C': '0x104', 'PWM_SRC': '0x120'}
for leaf, off in leaf_off.items():
    device_width = device_uints.get(f'CLKGEN_DIV_{leaf}_FACTOR_WIDTH')
    device_init = device_uints.get(f'CLKGEN_{leaf}_INITIAL_FACTOR')
    width, init = driver_div.get(int(off, 16), (None, None))
    ok = (device_width is not None and device_init is not None and
          device_width == width and device_init == init)
    print(f'  {leaf:<8} width {device_width} init {device_init} '
          f'driver(width {width}, init {init}): {"OK" if ok else "FAIL"}')
    if not ok:
        issues.append(f'div leaf {leaf} width/init')

print('=== 6. memory map: sgll *_BASE vs Table 3.4 ===')
trm_map = {}
b = blob(r'Table 3\.4: Memory mapping', r'3\.4 Other|Table 3\.5')
size_tok = r'(?:4KB?|8M|384K|15872K|16KB?|64KB?|128KB?|256KB?|1MB|256MB|64M)'
pairs = list(re.finditer(r'0x([0-9A-Fa-f]{8})\s+0x[0-9A-Fa-f]{8}\s*', b))
for i, m in enumerate(pairs):
    start_txt = b[m.end():pairs[i + 1].start() if i + 1 < len(pairs) else len(b)]
    name = re.sub(r'\s+', ' ', start_txt.strip())
    # Cut page-break header junk first ("able 3.4 - continued from previous
    # page Start Address [31:0] ..."), then the row's trailing size token.
    name = re.split(r'\b(?:continued|Start Address|End Address|Function Description|Size \(Byte\)|Reading and writing)\b|able \d\.\d|\d+\.\d+(?:\.\d+)? [A-Z]', name)[0].strip()
    name = re.sub(size_tok + r'\s*$', '', name).strip()
    if name:
        trm_map[int(m.group(1), 16)] = name
alias = {'WATCH DOG0': 'WDT0', 'WATCH DOG1': 'WDT1', 'WATCH DOG2': 'WDT2',
         'eMMC': 'EMMC', 'CLKGEN/PLL': 'CLKGEN', 'CryptoDMA': 'CRYPTO_DMA',
         'ap_mailbox': 'APB_MAILBOX', 'ap_system_ctrl': 'AP_SYSTEM_CTRL',
         'SPI NAND': 'SPI_NAND', 'I2S Global': 'I2S_GLOBAL',
         'RTCSYS_Timer': 'RTC_TIMER', 'RTCSYS_GPIO': 'RTC_GPIO',
         'RTCSYS_UART': 'RTC_UART', 'RTCSYS_INTR': 'RTC_INTR',
         'RTCSYS_MBOX': 'RTC_MBOX', 'RTCSYS_CTRL': 'RTC_CTRL',
         'RTCSYS_CORE': 'RTC_CORE', 'RTCSYS_IO': 'RTC_IO',
         'RTCSYS_OSC': 'RTC_OSC', 'RTCSYS_32kless': 'RTC_32KLESS',
         'RTCSYS_I2C': 'RTC_I2C', 'RTCSYS_SARADC': 'RTC_SARADC',
         'RTCSYS_WDT': 'RTC_WDT', 'PLIC': 'PLIC', 'DMA': 'DMA',
         'TIMER': 'TIMER', 'TEMPSEN': 'TEMPSEN', 'SARADC': 'SARADC',
         'USB': 'USB', 'AUDSRC': 'AUDSRC', 'WGN0': 'WGN0', 'WGN1': 'WGN1',
         'WGN2': 'WGN2', 'KEYSCAN': 'KEYSCAN', 'EFUSE': 'EFUSE',
         'I2S0': 'I2S0', 'I2S1': 'I2S1', 'I2S2': 'I2S2', 'I2S3': 'I2S3'}
checked = 0
SKIP_MAP = {'CLINT', 'RTCSYS_IRRX', 'IMG_V', 'IMG_D', 'SC_D', 'SC_V1',
            'SC_V2', 'SC_V3', 'DISP', 'DSI_MAC', 'CMDQ', 'DSI_PHY'}
for base, name in sorted(trm_map.items()):
    if 'Reserved' in name or alias.get(name, name.upper()) in SKIP_MAP:
        continue
    canon = alias.get(name, name.upper().replace(' ', '_').replace('/', '_'))
    mac = 'PLIC_BASE_ADDRESS' if canon == 'PLIC' else f'{canon}_BASE'
    val = sgll_defs.get(mac)
    if val is None:
        print(f'  (TRM {name} 0x{base:08X}: no {mac} - ok if out of scope)')
    elif val != base:
        print(f'  FAIL: {mac} sgll=0x{val:08X} TRM=0x{base:08X}')
        issues.append(f'base mismatch {mac}')
    else:
        checked += 1
extra = []
for mac, val in sgll_defs.items():
    suffix = '_BASE_ADDRESS' if mac.endswith('_BASE_ADDRESS') else '_BASE'
    canon = mac[:-len(suffix)]
    if canon == 'PLL_G2' and val == 0x03002800:
        checked += 1
        continue
    if canon == 'PLL_G6' and val == 0x03002900:
        checked += 1
        continue
    if val not in trm_map and canon not in ('PLL_G2', 'PLL_G6'):
        extra.append((mac, val))
for mac, val in extra:
    print(f'  NOTE sgll-only base {mac} 0x{val:08X} (check source)')
print(f'  sgll bases matched against TRM: {checked}/{len(sgll_defs)} '
      f'(PLL_G2/G6 from Tables 8.5/8.6)')

print('=== 7. C906L IRQ numbers: sgll vs SDK intr_conf.h ===')
irq_alias = {'SDMA_INTR_CPU2': 'SDMA', 'I2S0_INT': 'I2S0', 'I2S1_INT': 'I2S1',
             'I2S2_INT': 'I2S2', 'I2S3_INT': 'I2S3', 'UART0_INTR': 'UART0',
             'UART1_INTR': 'UART1', 'I2C0_INTR': 'I2C0', 'I2C1_INTR': 'I2C1',
             'I2C2_INTR': 'I2C2', 'I2C3_INTR': 'I2C3', 'I2C4_INTR': 'I2C4',
             'SPI_0_SSI_INTR': 'SPI0', 'SPI_1_SSI_INTR': 'SPI1',
             'WDT2_INTR': 'WDT2', 'KEYSCAN_IRQ': 'KEYSCAN',
             'GPIO0_INTR_FLAG': 'GPIO0', 'GPIO1_INTR_FLAG': 'GPIO1',
             'GPIO2_INTR_FLAG': 'GPIO2', 'GPIO3_INTR_FLAG': 'GPIO3',
             'WGN0_IRQ': 'WGN0', 'MBOX_INT1': 'MBOX1', 'IRRX_INT': 'IRRX',
             'GPIO_INT': 'RTC_GPIO', 'UART_INT': 'RTC_UART', 'I2C_INT': 'RTC_I2C',
             'WDT_INT': 'RTC_WDT', 'TDMA_INTERRUPT': 'TDMA',
             'SW_INT_0_CPU2': 'SW0_CPU2', 'SW_INT_1_CPU2': 'SW1_CPU2',
             'TIMER_INTR_4': 'TIMER4', 'TIMER_INTR_5': 'TIMER5',
             'TIMER_INTR_6': 'TIMER6', 'TIMER_INTR_7': 'TIMER7',
             'SPACC_IRQ': 'SPACC', 'IVE_INT': 'IVE',
             'MBOX_INT_C906_2ND': 'MBOX_C906_2ND'}
sdk_irq = {}
for m in re.finditer(r'#define\s+(\w+)\s+(\d+)\s*$', open(f'{SDK}/hal/cv181x/config/intr_conf.h').read(), re.M):
    if m.group(1) in irq_alias:
        sdk_irq[irq_alias[m.group(1)]] = int(m.group(2))
sgll_irq = {name.removeprefix('IRQ_'): value for name, value in device_uints.items()
            if name.startswith('IRQ_')}
bad = 0
for name, val in sorted(sdk_irq.items()):
    got = sgll_irq.get(name)
    if got is None:
        print(f'  FAIL: SDK routes {name}={val} to C906L, missing in sgll')
        issues.append(f'irq missing {name}')
        bad += 1
    elif got != val:
        print(f'  FAIL: {name} SDK={val} sgll={got}')
        issues.append(f'irq mismatch {name}')
        bad += 1
for name in sgll_irq:
    if name not in sdk_irq:
        print(f'  FAIL: sgll IRQ_{name} not routed to C906L in SDK')
        issues.append(f'irq not in SDK {name}')
        bad += 1
print(f'  SDK C906L-routed sources: {len(sdk_irq)}, sgll constants: {len(sgll_irq)}, bad: {bad}')

print()
print('=== RESULT ===')
if issues:
    print(f'{len(issues)} issue(s):')
    for i in issues:
        print(' -', i)
    sys.exit(1)
print('all checks passed')
