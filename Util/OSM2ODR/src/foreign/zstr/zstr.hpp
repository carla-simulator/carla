//---------------------------------------------------------
// Copyright 2015 Ontario Institute for Cancer Research
// Written by Matei David (matei@cs.toronto.edu)
//---------------------------------------------------------

// Reference:
// http://stackoverflow.com/questions/14086417/how-to-write-custom-input-stream-in-c

#ifndef __ZSTR_HPP
#define __ZSTR_HPP

#include <cassert>
#include <fstream>
#include <sstream>
#include <zlib.h>
#include "strict_fstream.hpp"

namespace zstr
{

/// Exception class thrown by failed zlib operations.
class Exception
    : public std::exception
{
public:
    Exception(z_stream * zstrm_p, int ret)
        : _msg("zlib: ")
    {
        switch (ret)
        {
        case Z_STREAM_ERROR:
            _msg += "Z_STREAM_ERROR: ";
            break;
        case Z_DATA_ERROR:
            _msg += "Z_DATA_ERROR: ";
            break;
        case Z_MEM_ERROR:
            _msg += "Z_MEM_ERROR: ";
            break;
        case Z_VERSION_ERROR:
            _msg += "Z_VERSION_ERROR: ";
            break;
        case Z_BUF_ERROR:
            _msg += "Z_BUF_ERROR: ";
            break;
        default:
            std::ostringstream oss;
            oss << ret;
            _msg += "[" + oss.str() + "]: ";
            break;
        }
        _msg += zstrm_p->msg;
    }
    Exception(const std::string msg) : _msg(msg) {}
    const char * what() const NOEXCEPT { return _msg.c_str(); }
private:
    std::string _msg;
}; // class Exception

namespace detail
{

class z_stream_wrapper
    : public z_stream
{
public:
    z_stream_wrapper(bool _is_input = true, int _level = Z_DEFAULT_COMPRESSION)
        : is_input(_is_input)
    {
        this->zalloc = Z_NULL;
        this->zfree = Z_NULL;
        this->opaque = Z_NULL;
        int ret;
        if (is_input)
        {
            this->avail_in = 0;
            this->next_in = Z_NULL;
            ret = inflateInit2(this, 15+32);
        }
        else
        {
            ret = deflateInit2(this, _level, Z_DEFLATED, 15+16, 8, Z_DEFAULT_STRATEGY);
        }
        if (ret != Z_OK) throw Exception(this, ret);
    }
    ~z_stream_wrapper()
    {
        if (is_input)
        {
            inflateEnd(this);
        }
        else
        {
            deflateEnd(this);
        }
    }
private:
    bool is_input;
}; // class z_stream_wrapper

} // namespace detail

class istreambuf
    : public std::streambuf
{
public:
    istreambuf(std::streambuf * _sbuf_p,
               std::size_t _buff_size = default_buff_size, bool _auto_detect = true)
        : sbuf_p(_sbuf_p),
          zstrm_p(nullptr),
          buff_size(_buff_size),
          auto_detect(_auto_detect),
          auto_detect_run(false),
          is_text(false)
    {
        assert(sbuf_p);
        in_buff = new char [buff_size];
        in_buff_start = in_buff;
        in_buff_end = in_buff;
        out_buff = new char [buff_size];
        setg(out_buff, out_buff, out_buff);
    }

    istreambuf(const istreambuf &) = delete;
    istreambuf & operator = (const istreambuf &) = delete;

    virtual ~istreambuf()
    {
        delete [] in_buff;
        delete [] out_buff;
        if (zstrm_p) delete zstrm_p;
    }

    virtual std::streambuf::int_type underflow()
    {
        if (this->gptr() == this->egptr())
        {
            // pointers for free region in output buffer
            char * out_buff_free_start = out_buff;
            do
            {
                // read more input if none available
                if (in_buff_start == in_buff_end)
                {
                    // empty input buffer: refill from the start
                    in_buff_start = in_buff;
                    std::streamsize sz = sbuf_p->sgetn(in_buff, buff_size);
                    in_buff_end = in_buff + sz;
                    if (in_buff_end == in_buff_start) break; // end of input
                }
                // auto detect if the stream contains text or deflate data
                if (auto_detect && ! auto_detect_run)
                {
                    auto_detect_run = true;
                    unsigned char b0 = *reinterpret_cast< unsigned char * >(in_buff_start);
                    unsigned char b1 = *reinterpret_cast< unsigned char * >(in_buff_start + 1);
                    // Ref:
                    // http://en.wikipedia.org/wiki/Gzip
                    // http://stackoverflow.com/questions/9050260/what-does-a-zlib-header-look-like
                    is_text = ! (in_buff_start + 2 <= in_buff_end
                                 && ((b0 == 0x1F && b1 == 0x8B)         // gzip header
                                     || (b0 == 0x78 && (b1 == 0x01      // zlib header
                                                        || b1 == 0x9C
                                                        || b1 == 0xDA))));
                }
                if (is_text)
                {
                    // simply swap in_buff and out_buff, and adjust pointers
                    assert(in_buff_start == in_buff);
                    std::swap(in_buff, out_buff);
                    out_buff_free_start = in_buff_end;
                    in_buff_start = in_buff;
                    in_buff_end = in_buff;
                }
                else
                {
                    // run inflate() on input
                    if (! zstrm_p) zstrm_p = new detail::z_stream_wrapper(true);
                    zstrm_p->next_in = reinterpret_cast< decltype(zstrm_p->next_in) >(in_buff_start);
                    zstrm_p->avail_in = (uInt)(in_buff_end - in_buff_start);
                    zstrm_p->next_out = reinterpret_cast< decltype(zstrm_p->next_out) >(out_buff_free_start);
                    zstrm_p->avail_out = (uInt)((out_buff + buff_size) - out_buff_free_start);
                    int ret = inflate(zstrm_p, Z_NO_FLUSH);
                    // process return code
                    if (ret != Z_OK && ret != Z_STREAM_END) throw Exception(zstrm_p, ret);
                    // update in&out pointers following inflate()
                    in_buff_start = reinterpret_cast< decltype(in_buff_start) >(zstrm_p->next_in);
                    in_buff_end = in_buff_start + zstrm_p->avail_in;
                    out_buff_free_start = reinterpret_cast< decltype(out_buff_free_start) >(zstrm_p->next_out);
                    assert(out_buff_free_start + zstrm_p->avail_out == out_buff + buff_size);
                    // if stream ended, deallocate inflator
                    if (ret == Z_STREAM_END)
                    {
                        delete zstrm_p;
                        zstrm_p = nullptr;
                    }
                }
            } while (out_buff_free_start == out_buff);
            // 2 exit conditions:
            // - end of input: there might or might not be output available
            // - out_buff_free_start != out_buff: output available
            this->setg(out_buff, out_buff, out_buff_free_start);
        }
        return this->gptr() == this->egptr()
            ? traits_type::eof()
            : traits_type::to_int_type(*this->gptr());
    }
private:
    std::streambuf * sbuf_p;
    char * in_buff;
    char * in_buff_start;
    char * in_buff_end;
    char * out_buff;
    detail::z_stream_wrapper * zstrm_p;
    std::size_t buff_size;
    bool auto_detect;
    bool auto_detect_run;
    bool is_text;

    static const std::size_t default_buff_size = (std::size_t)1 << 20;
}; // class istreambuf

class ostreambuf
    : public std::streambuf
{
public:
    ostreambuf(std::streambuf * _sbuf_p,
               std::size_t _buff_size = default_buff_size, int _level = Z_DEFAULT_COMPRESSION)
        : sbuf_p(_sbuf_p),
          zstrm_p(new detail::z_stream_wrapper(false, _level)),
          buff_size(_buff_size)
    {
        assert(sbuf_p);
        in_buff = new char [buff_size];
        out_buff = new char [buff_size];
        setp(in_buff, in_buff + buff_size);
    }

    ostreambuf(const ostreambuf &) = delete;
    ostreambuf & operator = (const ostreambuf &) = delete;

    int deflate_loop(int flush)
    {
        int ret = Z_OK;
        while (ret != Z_STREAM_END && ret != Z_BUF_ERROR)
        {
            zstrm_p->next_out = reinterpret_cast< decltype(zstrm_p->next_out) >(out_buff);
            zstrm_p->avail_out = (uInt)buff_size;
            ret = deflate(zstrm_p, flush);
            if (ret != Z_OK && ret != Z_STREAM_END && ret != Z_BUF_ERROR) throw Exception(zstrm_p, ret);
            std::streamsize sz = sbuf_p->sputn(out_buff, reinterpret_cast< decltype(out_buff) >(zstrm_p->next_out) - out_buff);
            if (sz != reinterpret_cast< decltype(out_buff) >(zstrm_p->next_out) - out_buff)
            {
                // there was an error in the sink stream
                return -1;
            }
            if (sz == 0)
            {
                break;
            }
        }
        return 0;
    }

    virtual ~ostreambuf()
    {
        // flush the zlib stream
        //
        // NOTE: Errors here (sync() return value not 0) are ignored, because we
        // cannot throw in a destructor. This mirrors the behaviour of
        // std::basic_filebuf::~basic_filebuf(). To see an exception on error,
        // close the ofstream with an explicit call to close(), and do not rely
        // on the implicit call in the destructor.
        //
        sync();
        delete [] in_buff;
        delete [] out_buff;
        delete zstrm_p;
    }
    virtual std::streambuf::int_type overflow(std::streambuf::int_type c = traits_type::eof())
    {
        zstrm_p->next_in = reinterpret_cast< decltype(zstrm_p->next_in) >(pbase());
        zstrm_p->avail_in = (uInt)(pptr() - pbase());
        while (zstrm_p->avail_in > 0)
        {
            int r = deflate_loop(Z_NO_FLUSH);
            if (r != 0)
            {
                setp(nullptr, nullptr);
                return traits_type::eof();
            }
        }
        setp(in_buff, in_buff + buff_size);
        return traits_type::eq_int_type(c, traits_type::eof()) ? traits_type::eof() : sputc((char)c);
    }
    virtual int sync()
    {
        // first, call overflow to clear in_buff
        overflow();
        if (! pptr()) return -1;
        // then, call deflate asking to finish the zlib stream
        zstrm_p->next_in = nullptr;
        zstrm_p->avail_in = 0;
        if (deflate_loop(Z_FINISH) != 0) return -1;
        deflateReset(zstrm_p);
        return 0;
    }
private:
    std::streambuf * sbuf_p;
    char * in_buff;
    char * out_buff;
    detail::z_stream_wrapper * zstrm_p;
    std::size_t buff_size;

    static const std::size_t default_buff_size = (std::size_t)1 << 20;
}; // class ostreambuf

class istream
    : public std::istream
{
public:
    istream(std::istream & is)
        : std::istream(new istreambuf(is.rdbuf()))
    {
        exceptions(std::ios_base::badbit);
    }
    explicit istream(std::streambuf * sbuf_p)
        : std::istream(new istreambuf(sbuf_p))
    {
        exceptions(std::ios_base::badbit);
    }
    virtual ~istream()
    {
        delete rdbuf();
    }
}; // class istream

class ostream
    : public std::ostream
{
public:
    ostream(std::ostream & os)
        : std::ostream(new ostreambuf(os.rdbuf()))
    {
        exceptions(std::ios_base::badbit);
    }
    explicit ostream(std::streambuf * sbuf_p)
        : std::ostream(new ostreambuf(sbuf_p))
    {
        exceptions(std::ios_base::badbit);
    }
    virtual ~ostream()
    {
        delete rdbuf();
    }
}; // class ostream

namespace detail
{

template < typename FStream_Type >
struct strict_fstream_holder
{
    strict_fstream_holder(const std::string& filename, std::ios_base::openmode mode = std::ios_base::in)
        : _fs(filename, mode)
    {}
    FStream_Type _fs;
}; // class strict_fstream_holder

} // namespace detail

class ifstream
    : private detail::strict_fstream_holder< strict_fstream::ifstream >,
      public std::istream
{
public:
    explicit ifstream(const std::string& filename, std::ios_base::openmode mode = std::ios_base::in)
        : detail::strict_fstream_holder< strict_fstream::ifstream >(filename, mode),
          std::istream(new istreambuf(_fs.rdbuf()))
    {
        exceptions(std::ios_base::badbit);
    }
    virtual ~ifstream()
    {
        if (rdbuf()) delete rdbuf();
    }
}; // class ifstream

class ofstream
    : private detail::strict_fstream_holder< strict_fstream::ofstream >,
      public std::ostream
{
public:
    explicit ofstream(const std::string& filename, std::ios_base::openmode mode = std::ios_base::out)
        : detail::strict_fstream_holder< strict_fstream::ofstream >(filename, mode | std::ios_base::binary),
          std::ostream(new ostreambuf(_fs.rdbuf()))
    {
        exceptions(std::ios_base::badbit);
    }
    virtual ~ofstream()
    {
        if (rdbuf()) delete rdbuf();
    }
}; // class ofstream

} // namespace zstr

#endif
