#!/bin/sh

# Command Line Options
# -TPPK3 - Specifies PicKit3
# -P<Target Device>
# -F<HexFile>
# -M<program memory>
# -B (Batch mode)
# -L (Release MCLR after programming)
IPE=/opt/microchip/mplabx/v6.15/mplab_platform/mplab_ipe/ipecmd.sh
mplabx_dir=/opt/microchip/mplabx/v6.15/mplab_platform
netbeans_dir=$mplabx_dir
ipecmd_jar=$mplabx_dir/mplab_ipe/ipecmd.jar
. $mplabx_dir/etc/mplab_ipe.conf
jvm=$jdkhome/bin/java
$jvm -jar $ipecmd_jar -P16F1937 -TPPK3 -F"Firmware.hex" -M -OB
