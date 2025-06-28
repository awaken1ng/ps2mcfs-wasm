export type McFatCardSpecs = {
  pageSize: number,
  blockSize: number,
  cardSize: number,
  cardFlags: number,
}

export type McStDateTime = {
  resv2: number,
  sec: number,
  min: number,
  hour: number,
  day: number,
  month: number,
  year: number,
}

export type McIoStat = {
  mode: number,
  attr: number,
  size: number,
  ctime: McStDateTime,
  mtime: McStDateTime,
}

export type McEntryInfo = {
  stat: McIoStat,
  name: string,
}

export type McEntry = McEntryInfo & {
  contents: Uint8Array,
}

export type McDirEntry = McEntryInfo & {
  hasMore: boolean,
}

export type McReturnCode = number
export type McFileHandle = number

export enum SeekOrigin {
  Set = 0,
  Current = 1,
  End = 2,
}

export type McResultCode = { code: number }
export type McResultGetInfo = McResultCode | McFatCardSpecs
export type McResultGetAvailableSpace = McResultCode | { availableSpace: number }
export type McResultData = McResultCode | { data: Uint8Array }
export type McResultReadDir = McResultCode | McDirEntry
export type McResultStat = McResultCode | McEntryInfo

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

export const sceMcFileAttrReadable = 0x0001;
export const sceMcFileAttrWriteable = 0x0002;
export const sceMcFileAttrExecutable = 0x0004;
export const sceMcFileAttrDupProhibit = 0x0008;
export const sceMcFileAttrFile = 0x0010;
export const sceMcFileAttrSubdir = 0x0020;
export const sceMcFileCreateDir = 0x0040;
export const sceMcFileAttrClosed = 0x0080;
export const sceMcFileCreateFile = 0x0200;
export const sceMcFile0400 = 0x0400;
export const sceMcFileAttrPDAExec = 0x0800;
export const sceMcFileAttrPS1 = 0x1000;
export const sceMcFileAttrHidden = 0x2000;
export const sceMcFileAttrExists = 0x8000;

export const CF_USE_ECC = 0x01
export const CF_BAD_BLOCK = 0x08
export const CF_ERASE_ZEROES = 0x10

export const mcFileUpdateName: number;
export const mcFileUpdateAttrCtime: number;
export const mcFileUpdateAttrMtime: number;
export const mcFileUpdateAttrMode: number;

export interface Module {
  setCardBuffer(buffer: Uint8Array): void;
  getCardBuffer(): Uint8Array;
  generateCardBuffer(): void;
  setCardSpecs(specs: McFatCardSpecs): void;
  setCardChanged(v: boolean): void;

  init(): McReturnCode;
  detect(): McReturnCode;
  close(fd: McFileHandle): McReturnCode;
  createCrossLinkedFile(realFilePath: string, dummyFilePath: string): McReturnCode;
  dclose(fd: McFileHandle): McReturnCode;
  unformat(): McReturnCode;
  format(): McReturnCode;
  remove(fileName: string): McReturnCode;
  rmDir(dirName: string): McReturnCode;
  /** `mcFileUpdateName` - sets name, does NOT check if file/directory with same name already exists */
  /** `mcFileUpdateAttrCtime` - sets create date */
  /** `mcFileUpdateAttrMtime` - sets modified date */
  /** `mcFileUpdateAttrMode` - sets attributes */
  setInfo(fileName: string, stat: McIoStat, name: string, flags: number): McReturnCode;

  open(fileName: string, flag: number): McReturnCode | McFileHandle;
  write(fd: McFileHandle, data: Uint8Array): McReturnCode | number;
  seek(fd: McFileHandle, offset: number, origin: SeekOrigin): McReturnCode | number;
  getCluster(fd: McFileHandle): McReturnCode | number;
  dopen(dirName: string): McReturnCode | McFileHandle;
  mkDir(dirName: string): McReturnCode | McFileHandle;

  getInfo(): McResultGetInfo;
  getAvailableSpace(): McResultGetAvailableSpace;
  read(fd: McFileHandle, length: number): McResultData;
  dread(fd: McFileHandle): McResultReadDir;
  readPage(pageIdx: number): McResultData;
  stat(fd: McFileHandle): McResultStat;
}

export default function createModule(options?: unknown): Promise<Module>
