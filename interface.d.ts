type Uint8Buffer = ArrayBuffer | Uint8Array | Uint8ClampedArray

export type McFatCardSpecs = {
  pageSize: number,
  blockSize: number,
  cardSize: number,
  cardFlags: number
}

export type McStDateTime = {
  resv2: number,
  sec: number,
  min: number,
  hour: number,
  day: number,
  month: number,
  year: number
}

export type McIoStat = {
  mode: number,
  attr: number,
  size: number,
  ctime: McStDateTime,
  mtime: McStDateTime
};

export type McReturnCode = number
export type McFileHandle = number

export type SeekSet = 0
export type SeekCurrent = 1
export type SeekEnd = 2
export type SeekOrigin = SeekSet | SeekCurrent | SeekEnd

export type McResultCode = { code: number }
export type McResultGetInfo = McResultCode | McFatCardSpecs
export type McResultGetAvailableSpace = McResultCode | { availableSpace: number }
export type McResultData = McResultCode | { data: Array<number> }
export type McResultReadDir = McResultCode | { hasMore: boolean, stat: McIoStat, name: string }

export interface Module {
  setBuffer(buffer: Uint8Buffer): void;
  getBuffer(): Array<number>;
  setCardSpecs(specs: McFatCardSpecs): void;
  setCardChanged(v: boolean): void;

  init(): McReturnCode;
  detect(): McReturnCode;
  close(fd: McFileHandle): McReturnCode;
  createCrossLinkedFile(realFilePath: string, dummyFilePath: string): McReturnCode;
  dclose(_fd: McFileHandle): McReturnCode;
  unformat(): McReturnCode;
  format(): McReturnCode;
  remove(fileName: string): McReturnCode;
  rmDir(dirName: string): McReturnCode;

  open(fileName: string, flag: number): McReturnCode | McFileHandle;
  write(fd: McFileHandle, data: Uint8Buffer): McReturnCode | number;
  seek(fd: McFileHandle, offset: number, origin: SeekOrigin): McReturnCode | number;
  getCluster(fd: McFileHandle): McReturnCode | number;
  dopen(dirName: string): McReturnCode | McFileHandle;
  mkDir(dirName: string): McReturnCode | McFileHandle;

  getInfo(): McResultGetInfo;
  getAvailableSpace(): McResultGetAvailableSpace;
  read(fd: McFileHandle, length: number): McResultData;
  dread(fd: McFileHandle): McResultReadDir;
  readPage(pageIdx: number): McResultData;
}

export default function MainModuleFactory (options?: unknown): Promise<Module>;
