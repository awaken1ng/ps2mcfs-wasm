#include <emscripten/emscripten.h>
#include <emscripten/bind.h>

#include "mcfat.h"
#include "mcio.h"

#include <cstdint>
#include <stdio.h>
#include <string>
#include <vector>

std::vector<uint8_t> buffer;

void setBuffer(emscripten::val v) {
    buffer = emscripten::vecFromJSArray<uint8_t>(v);
}

emscripten::val getBuffer() {
    return emscripten::val().array(buffer);
}

int page_erase(mcfat_cardspecs_t *specs, uint32_t page_idx) {
    bool use_ecc = (specs->flags & CF_USE_ECC) ? true : false;
    uint16_t page_size = specs->pagesize;
    uint16_t spare_size = page_size >> 5;
    uint16_t page_size_with_ecc = specs->pagesize + (use_ecc ? spare_size : 0);
	int erase_value = (specs->flags & CF_ERASE_ZEROES) ? 0x00 : 0xFF;
    uint8_t *buffer_ptr = buffer.data();

    memset(&buffer_ptr[page_idx * page_size_with_ecc], erase_value, page_size);

    uint8_t *p_ecc;
	int32_t size;
    if (use_ecc) {
        uint8_t* tmp_ecc = (uint8_t*) calloc(1, spare_size);
        p_ecc = tmp_ecc;
        size = 0;

        while (size < page_size) {
            Card_DataChecksum(&buffer_ptr[page_idx * page_size_with_ecc] + size, p_ecc);
            size += 128;
            p_ecc += 3;
        }

        memcpy(&buffer_ptr[page_idx * page_size_with_ecc + page_size], tmp_ecc, spare_size);
        free(tmp_ecc);
    }

    return sceMcResSucceed;
}

int page_write(mcfat_cardspecs_t *specs, uint32_t page_idx, void* buf) {
    bool use_ecc = (specs->flags & CF_USE_ECC) ? true : false;
    uint16_t page_size = specs->pagesize;
    uint16_t spare_size = page_size >> 5;
    uint16_t page_size_with_ecc = specs->pagesize + (use_ecc ? spare_size : 0);
	memcpy(&buffer.data()[page_idx * page_size_with_ecc], buf, specs->pagesize);
    return sceMcResSucceed;
}

int page_read(mcfat_cardspecs_t *specs, uint32_t page_idx, void* buf) {
    bool use_ecc = (specs->flags & CF_USE_ECC) ? true : false;
    uint16_t page_size = specs->pagesize;
    uint16_t spare_size = page_size >> 5;
    uint16_t page_size_with_ecc = specs->pagesize + (use_ecc ? spare_size : 0);
	memcpy(buf, &buffer.data()[page_idx * page_size_with_ecc], page_size);
    return sceMcResSucceed;
}

int ecc_write(mcfat_cardspecs_t *specs, uint32_t page_idx, void* buf) {
    bool use_ecc = (specs->flags & CF_USE_ECC) ? true : false;
    uint16_t page_size = specs->pagesize;
    uint16_t spare_size = page_size >> 5;
    uint16_t page_size_with_ecc = specs->pagesize + (use_ecc ? spare_size : 0);
    memcpy(&buffer.data()[page_idx * page_size_with_ecc + page_size], buf, spare_size);
    return sceMcResSucceed;

}

int ecc_read(mcfat_cardspecs_t *specs, uint32_t page_idx, void* buf) {
    bool use_ecc = (specs->flags & CF_USE_ECC) ? true : false;
    uint16_t page_size = specs->pagesize;
    uint16_t spare_size = page_size >> 5;
    uint16_t page_size_with_ecc = specs->pagesize + (use_ecc ? spare_size : 0);
    memcpy(buf, &buffer.data()[page_idx * page_size_with_ecc + page_size], spare_size);
    return sceMcResSucceed;
}

mcfat_mcops_t mcfat_mcops {
    .page_erase = page_erase,
    .page_write = page_write,
    .page_read = page_read,
    .ecc_write = ecc_write,
    .ecc_read = ecc_read,
};

void setCardSpecs(const mcfat_cardspecs_t cardspecs) {
    mcfat_setConfig(mcfat_mcops, cardspecs);
}

emscripten::val getInfo() {
    int pagesize, blocksize, cardsize, cardflags;
    int return_code = mcio_mcGetInfo(&pagesize, &blocksize, &cardsize, &cardflags);

    emscripten::val ret = emscripten::val::object();
    ret.set("returnCode", return_code);

    if (return_code == sceMcResSucceed) {
        emscripten::val data = emscripten::val().object();
        data.set("pageSize", pagesize);
        data.set("blockSize", blocksize);
        data.set("cardSize", cardsize);
        data.set("cardFlags", cardflags);

        ret.set("data", data);
    }

    return ret;
}

emscripten::val getAvailableSpace() {
    int cardfree;
    int return_code = mcio_mcGetAvailableSpace(&cardfree);

    emscripten::val ret = emscripten::val::object();
    ret.set("returnCode", return_code);

    if (return_code == sceMcResSucceed) {
        emscripten::val data = emscripten::val().object();
        data.set("availableSpace", cardfree);

        ret.set("data", data);
    }

    return ret;
}

int openFile(std::string filename, int flag) {
    return mcio_mcOpen(filename.data(), flag);
}

emscripten::val readFile(int fd, int length) {
    emscripten::val ret = emscripten::val::object();

    std::vector<uint8_t> buffer = std::vector<uint8_t>(length);
    int return_code = mcio_mcRead(fd, buffer.data(), buffer.size());
    if (return_code < 0) {
        ret.set("returnCode", return_code);
        return ret;
    }

    if (return_code != length) {
        ret.set("returnCode", sceMcResFailIO);
        return ret;
    }

    ret.set("data", emscripten::val::array(buffer));
    return ret;
}

int writeFile(int fd, emscripten::val data) {
    std::vector<uint8_t> vec = emscripten::vecFromJSArray<uint8_t>(data);

    return mcio_mcWrite(fd, vec.data(), vec.size());
}

int createCrossLinkedFile(std::string real_filepath, std::string dummy_filepath) {
    return mcio_mcCreateCrossLinkedFile(real_filepath.data(), dummy_filepath.data());
}

int openDirectory(std::string dirname) {
    return mcio_mcDopen(dirname.data());
}

emscripten::val readDirectory(int fd) {
    emscripten::val ret = emscripten::val::object();
    io_dirent dirent;
    int return_code = mcio_mcDread(fd, &dirent);
    if (return_code < 0) {
        ret.set("returnCode", return_code);
        return ret;
    }

    emscripten::val ctime = emscripten::val::object();
    ctime.set("resv2", dirent.stat.ctime.Resv2);
    ctime.set("sec", dirent.stat.ctime.Sec);
    ctime.set("min", dirent.stat.ctime.Min);
    ctime.set("hour", dirent.stat.ctime.Hour);
    ctime.set("day", dirent.stat.ctime.Day);
    ctime.set("month", dirent.stat.ctime.Month);
    ctime.set("year", dirent.stat.ctime.Year);

    emscripten::val mtime = emscripten::val::object();
    mtime.set("resv2", dirent.stat.mtime.Resv2);
    mtime.set("sec", dirent.stat.mtime.Sec);
    mtime.set("min", dirent.stat.mtime.Min);
    mtime.set("hour", dirent.stat.mtime.Hour);
    mtime.set("day", dirent.stat.mtime.Day);
    mtime.set("month", dirent.stat.mtime.Month);
    mtime.set("year", dirent.stat.mtime.Year);


    emscripten::val stat = emscripten::val::object();
    stat.set("mode", dirent.stat.attr);
    stat.set("attr", dirent.stat.attr);
    stat.set("size", dirent.stat.size);
    stat.set("ctime", ctime);
    stat.set("mtime", mtime);

    emscripten::val data = emscripten::val::object();
    data.set("stat", stat);
    data.set("name", dirent.name);

    ret.set("data", data);

    return ret;
}

int createDirectory(std::string dirname) {
    return mcio_mcMkDir(dirname.data());
}

emscripten::val readPage(int pageIdx) {
    emscripten::val ret = emscripten::val::object();
    int pagesize, _blocksize, _cardsize, _cardflags;
    int return_code = mcio_mcGetInfo(&pagesize, &_blocksize, &_cardsize, &_cardflags);
    if (return_code < 0) {
        ret.set("returnCode", return_code);
        return ret;
    }

    std::vector<uint8_t> page = std::vector<uint8_t>(pagesize);
    return_code = mcio_mcReadPage(pageIdx, page.data());
    if (return_code < 0) {
        ret.set("returnCode", return_code);
        return ret;
    }

    ret.set("page", page);

    return ret;
}

int deleteFile(std::string filename) {
    return mcio_mcRemove(filename.data());
}

int deleteDirectory(std::string dirname) {
    return mcio_mcRmDir(dirname.data());
}

EMSCRIPTEN_BINDINGS(mcfs) {
    // mcfat
    emscripten::value_object<mcfat_cardspecs_t>("McFatCardSpecs")
        .field("pageSize", &mcfat_cardspecs_t::pagesize)
        .field("blockSize", &mcfat_cardspecs_t::blocksize)
        .field("cardSize", &mcfat_cardspecs_t::cardsize)
        .field("cardFlags", &mcfat_cardspecs_t::flags);

    emscripten::function("mcfatSetBuffer", &setBuffer);
    emscripten::function("mcfatGetBuffer", &getBuffer);
    emscripten::function("mcfatSetCardSpecs", &setCardSpecs);
    emscripten::function("mcfatSetCardChanged", &mcfat_setCardChanged);

    // mcio
    emscripten::value_object<sceMcStDateTime>("McStDateTime")
        .field("resv2", &sceMcStDateTime::Resv2)
        .field("sec", &sceMcStDateTime::Sec)
        .field("min", &sceMcStDateTime::Min)
        .field("hour", &sceMcStDateTime::Hour)
        .field("day", &sceMcStDateTime::Day)
        .field("month", &sceMcStDateTime::Month)
        .field("year", &sceMcStDateTime::Year);

    emscripten::value_object<io_stat>("McIoStat")
        .field("mode", &io_stat::mode)
        .field("attr", &io_stat::attr)
        .field("size", &io_stat::size)
        .field("ctime", &io_stat::ctime)
        .field("mtime", &io_stat::mtime);

    emscripten::value_object<io_dirent>("McIoDirent")
        .field("stat", &io_dirent::stat)
        .field("name", &io_dirent::name)
        .field("unknown", &io_dirent::unknown);

    emscripten::function("mcioInit", &mcio_init);
    emscripten::function("mcioMcDetect", &mcio_mcDetect);
    emscripten::function("mcioMcGetInfo", &getInfo);
    emscripten::function("mcioMcGetAvailableSpace", &getAvailableSpace);
    emscripten::function("mcioMcOpen", &openFile);
    emscripten::function("mcioMcClose", &mcio_mcClose);
    emscripten::function("mcioMcRead", &readFile);
    emscripten::function("mcioMcWrite", &writeFile);
    emscripten::function("mcioMcSeek", &mcio_mcSeek);
    emscripten::function("mcioMcGetCluster", &mcio_mcGetCluster);
    emscripten::function("mcioMcCreateCrossLinkedFile", &createCrossLinkedFile);
    emscripten::function("mcioMcDopen", &openDirectory);
    emscripten::function("mcioMcDclose", &mcio_mcDclose);
    emscripten::function("mcioMcDread", &readDirectory);
    emscripten::function("mcioMcMkDir", &createDirectory);
    emscripten::function("mcioMcReadPage", &readPage);
    emscripten::function("mcioMcUnformat", &mcio_mcUnformat);
    emscripten::function("mcioMcFormat", &mcio_mcFormat);
    emscripten::function("mcioMcRemove", &deleteFile);
    emscripten::function("mcioMcRmDir", &deleteDirectory);
}
