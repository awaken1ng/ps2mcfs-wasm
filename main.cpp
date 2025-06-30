#include <cstring>
#include <emscripten/emscripten.h>
#include <emscripten/bind.h>

#include "mcfat.h"
#include "mcio.h"
#include "cardgen.h"

#include <cstdint>
#include <stdbool.h>
#include <stdio.h>
#include <string>
#include <vector>

std::vector<uint8_t> buffer;

void setCardBuffer(emscripten::val v) {
    buffer = std::vector<uint8_t>(); // deallocate the existing buffer first
    buffer = emscripten::convertJSArrayToNumberVector<uint8_t>(v);
}

void generateCardBuffer() {
    uint32_t cardSize = 8 * 1024 * 1024;

    buffer = std::vector<uint8_t>(); // deallocate the existing buffer first
    buffer.resize(cardSize);

    for (size_t pos = 0; pos < cardSize; pos += BLOCK_SIZE) {
        uint8_t *flushbuf = buffer.data() + pos;
        genblock(cardSize, pos, flushbuf);
    }
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

emscripten::val getCardSpecs() {
    int pagesize, blocksize, cardsize, cardflags;
    int code = mcio_mcGetCardSpecs(&pagesize, &blocksize, &cardsize, &cardflags);

    emscripten::val ret = emscripten::val::object();
    if (code == sceMcResSucceed) {
        emscripten::val data = emscripten::val::object();
        ret.set("pageSize", pagesize);
        ret.set("blockSize", blocksize);
        ret.set("cardSize", cardsize);
        ret.set("cardFlags", cardflags);
    } else {
        ret.set("code", code);
    }

    return ret;
}

emscripten::val getAvailableSpace() {
    int cardfree;
    int code = mcio_mcGetAvailableSpace(&cardfree);

    emscripten::val ret = emscripten::val::object();
    if (code == sceMcResSucceed) {
        ret.set("availableSpace", cardfree);
    } else {
        ret.set("code", code);
    }

    return ret;
}

int openFile(std::string filename, int flag) {
    return mcio_mcOpen(filename.data(), flag);
}

emscripten::val readFile(int fd, int length) {
    emscripten::val ret = emscripten::val::object();

    std::vector<uint8_t> buffer = std::vector<uint8_t>(length);
    int code = mcio_mcRead(fd, buffer.data(), buffer.size());
    if (code < 0) {
        ret.set("code", code);
    } else {
        ret.set("data", emscripten::val::global("Uint8Array").new_(emscripten::val::array(buffer)));
    }

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
    int code = mcio_mcDread(fd, &dirent);
    if (code < 0) {
        ret.set("code", code);
    } else {
        ret.set("hasMore", code == 1);
        ret.set("stat", dirent.stat);
        ret.set("name", dirent.name);
    }

    return ret;
}

int createDirectory(std::string dirname) {
    return mcio_mcMkDir(dirname.data());
}

emscripten::val readPage(int pageIdx, bool withEcc) {
    emscripten::val ret = emscripten::val::object();
    int pagesize, _blocksize, _cardsize, _cardflags;
    int code = mcio_mcGetCardSpecs(&pagesize, &_blocksize, &_cardsize, &_cardflags);
    if (code < 0) {
        ret.set("code", code);
        return ret;
    }

    emscripten::val uint8array = emscripten::val::global("Uint8Array");
    std::vector<uint8_t> page = std::vector<uint8_t>(pagesize);

    if (withEcc) {
        std::vector<uint8_t> ecc = std::vector<uint8_t>(pagesize >> 5);
        code = mcio_mcReadPage(pageIdx, page.data(), ecc.data());

        if (code < 0) {
            ret.set("code", code);
        } else {
            ret.set("data", uint8array.new_(emscripten::val::array(page)));
            ret.set("ecc", uint8array.new_(emscripten::val::array(ecc)));
        }
    } else {
        code = mcio_mcReadPage(pageIdx, page.data(), NULL);

        if (code < 0) {
            ret.set("code", code);
        } else {
            ret.set("data", uint8array.new_(emscripten::val::array(page)));
        }
    }

    return ret;
}

int deleteFile(std::string filename) {
    return mcio_mcRemove(filename.data());
}

int deleteDirectory(std::string dirname) {
    return mcio_mcRmDir(dirname.data());
}

int setInfo(std::string filename, io_stat stat, std::string name, int flags) {
    io_dirent info = {};
    info.stat = stat;
    strncpy(info.name, name.data(), 32);
    return mcio_mcSetInfo(filename.data(), &info, flags);
}

emscripten::val stat(int fd) {
    emscripten::val ret = emscripten::val::object();
    io_dirent info = {};
    int code = mcio_mcStat(fd, &info);
    if (code < 0) {
        ret.set("code", code);
    } else {
        ret.set("stat", info.stat);
        ret.set("name", info.name);
    }

    return ret;
}

EMSCRIPTEN_BINDINGS(mcfs) {
    // mcfat
    emscripten::value_object<mcfat_cardspecs_t>("McFatCardSpecs")
        .field("pageSize", &mcfat_cardspecs_t::pagesize)
        .field("blockSize", &mcfat_cardspecs_t::blocksize)
        .field("cardPages", &mcfat_cardspecs_t::cardpages)
        .field("cardFlags", &mcfat_cardspecs_t::flags);

    emscripten::function("setCardBuffer", &setCardBuffer);
    emscripten::function("generateCardBuffer", &generateCardBuffer);
    emscripten::function("setCardSpecs", &setCardSpecs);
    emscripten::function("setCardChanged", &mcfat_setCardChanged);

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

    // returns just status code
    emscripten::function("init", &mcio_init);
    emscripten::function("detect", &mcio_mcDetect);
    emscripten::function("close", &mcio_mcClose);
    emscripten::function("createCrossLinkedFile", &createCrossLinkedFile);
    emscripten::function("dclose", &mcio_mcDclose);
    emscripten::function("unformat", &mcio_mcUnformat);
    emscripten::function("format", &mcio_mcFormat);
    emscripten::function("remove", &deleteFile);
    emscripten::function("rmDir", &deleteDirectory);
    emscripten::function("setInfo", &setInfo);

    // returns status code (if <0) or data (if >=0)
    emscripten::function("open", &openFile);
    emscripten::function("write", &writeFile);
    emscripten::function("seek", &mcio_mcSeek);
    emscripten::function("getCluster", &mcio_mcGetCluster);
    emscripten::function("dopen", &openDirectory);
    emscripten::function("mkDir", &createDirectory);

    // returns object with status code or status code and data
    emscripten::function("getCardSpecs", &getCardSpecs);
    emscripten::function("getAvailableSpace", &getAvailableSpace);
    emscripten::function("read", &readFile);
    emscripten::function("dread", &readDirectory);
    emscripten::function("readPage", &readPage);
    emscripten::function("stat", &stat);
}
