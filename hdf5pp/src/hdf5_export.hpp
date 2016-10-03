#ifndef HDF5_EXPORT_HPP
#define HDF5_EXPORT_HPP
#include "hdf5.hpp"
#include "hdf5_impl.hpp"

namespace think { namespace hdf5 {
  class library {
  public:

    static void initLibrary() { H5Library::open(); H5Library::initH5cpp(); }
    static void termH5cpp() { H5Library::termH5cpp(); H5Library::close(); }
    static void getLibVersion( unsigned& majnum, unsigned& minnum, unsigned& relnum )
    {
      H5Library::getLibVersion( majnum, minnum, relnum );
    }

    static const object* open_file( const char* name, int access )
    {
      return new file( name, access );
    }

    static void close_file( object* file )
    {
      delete file;
    }
  };

}}
#endif
