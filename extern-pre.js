/* eslint-disable */

export const sceMcResSucceed = 0
export const sceMcResChangedCard = -1
export const sceMcResNoFormat = -2
export const sceMcResFullDevice = -3
export const sceMcResNoEntry = -4
export const sceMcResDeniedPermit = -5
export const sceMcResNotEmpty = -6
export const sceMcResUpLimitHandle = -7
export const sceMcResFailReplace = -8
export const sceMcResFailResetAuth = -11
export const sceMcResFailDetect = -12
export const sceMcResFailDetect2 = -13
export const sceMcResFailReadCluster = -21
export const sceMcResFailCheckBackupBlocks = -47
export const sceMcResFailIO = -48
export const sceMcResFailSetDeviceSpecs = -49
export const sceMcResDeniedPS1Permit = -51
export const sceMcResFailAuth = -90
export const sceMcResNotDir = -100
export const sceMcResNotFile = -101

export const sceMcFileAttrReadable = 0x0001
export const sceMcFileAttrWriteable = 0x0002
export const sceMcFileAttrExecutable = 0x0004
export const sceMcFileAttrDupProhibit = 0x0008
export const sceMcFileAttrFile = 0x0010
export const sceMcFileAttrSubdir = 0x0020
export const sceMcFileCreateDir = 0x0040
export const sceMcFileAttrClosed = 0x0080
export const sceMcFileCreateFile = 0x0200
export const sceMcFile0400 = 0x0400
export const sceMcFileAttrPDAExec = 0x0800
export const sceMcFileAttrPS1 = 0x1000
export const sceMcFileAttrHidden = 0x2000
export const sceMcFileAttrExists = 0x8000

export const CF_USE_ECC = 0x01
export const CF_BAD_BLOCK = 0x08
export const CF_ERASE_ZEROES = 0x10

export const mcFileUpdateName = sceMcFileAttrFile
export const mcFileUpdateAttrCtime = sceMcFileAttrReadable
export const mcFileUpdateAttrMtime = sceMcFileAttrWriteable
export const mcFileUpdateAttrMode = 0x10000
