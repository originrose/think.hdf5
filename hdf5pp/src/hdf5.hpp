#ifndef HDF5HPP
#define HDF5HPP

#include <H5Cpp.h>
#include <string>
#include <vector>
#include <memory>


namespace think { namespace hdf5 {
  using namespace H5;
  using namespace std;

  struct Access {
    enum EEnum {
      excl = 1 << 0,
      trunc = 1 << 1,
      rdonly = 1 << 2,
      rdrw = 1 << 3,
      debug = 1 << 4,
      create = 1 << 5,
    };
  };

  struct EObjType {
    enum EEnum {
      unknown = -1,
      group,
      dataset,
      datatype,
      file,
    };
  };

  class object;
  typedef vector<object*> obj_ptr_list;
  typedef shared_ptr<object> shared_obj_ptr;
  typedef vector<shared_obj_ptr> shared_obj_ptr_list;

  class object
  {
  public:
    virtual ~object() {}
    virtual EObjType::EEnum type() const = 0;
    virtual string name() const  = 0;
    virtual obj_ptr_list children() const = 0;
  protected:
    static obj_ptr_list convert( const shared_obj_ptr_list& data )
    {
      obj_ptr_list retval( data.size() );
      for ( size_t idx = 0; idx < data.size(); ++idx ) {
	retval[idx] = data[idx].get();
      }
      return retval;
    }
  };


}}

#endif
