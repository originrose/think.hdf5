#ifndef HDF5_IMPL_H
#define HDF5_IMPL_H

#include "hdf5.hpp"
#include <H5Fpublic.h>

namespace think { namespace hdf5 {

  class rand_obj : public object
  {
    string m_name;
    shared_obj_ptr_list m_children;
    EObjType::EEnum m_type;

  public:
    rand_obj( string& name, EObjType::EEnum type )
      : m_name (name )
      , m_type( type )
    {
    }
    virtual EObjType::EEnum type() const { return m_type; }
    virtual string name() const { return m_name; }
    virtual obj_ptr_list children() const
    { return convert( m_children ); }

  protected:
  };


  class common_fg : public object
  {
    CommonFG& m_file_or_group;
    EObjType::EEnum m_type;
    mutable shared_obj_ptr_list m_children;
    string m_name;
  public:
    common_fg( CommonFG& file_or_group, EObjType::EEnum type, const string& name)
      : m_file_or_group( file_or_group )
      , m_type( type )
      , m_name( name )
    {
    }
    EObjType::EEnum type() const { return m_type; }
    string name() const { return m_name; }
    virtual obj_ptr_list children() const
    {
      if ( m_children.empty() ) {
	size_t num_objs = m_file_or_group.getNumObjs();
	for ( size_t idx = 0; idx < num_objs; ++idx ) {
	  string child_name = m_file_or_group.getObjnameByIdx( idx );
	  int child_type = m_file_or_group.childObjType( idx );
	  m_children.push_back(
	    shared_obj_ptr( new rand_obj( child_name
					  , static_cast<EObjType::EEnum>( child_type ) ) ) );
	}
      }
      return convert( m_children );
    }
  };

class file : public object
{
  H5File m_file;
  common_fg m_common_fg;
  static int to_hdf5_access( int access ) {
    int retval = 0;
    if ( access & Access::excl ) retval |= H5F_ACC_EXCL;
    if ( access & Access::trunc ) retval |= H5F_ACC_TRUNC;
    if ( access & Access::rdonly ) retval |= H5F_ACC_RDONLY;
    if ( access & Access::rdrw ) retval |= H5F_ACC_RDWR;
    if ( access & Access::debug ) retval |= H5F_ACC_DEBUG;
    if ( access & Access::create ) retval |= H5F_ACC_CREAT;
    return retval;
  }
public:
  file( const char* name, unsigned int flags )
    : m_file( name, to_hdf5_access( flags ) )
    , m_common_fg( m_file, EObjType::file, name )
  {
  }
  static bool is_hdf5_file( const char* name )
  {
    return H5File::isHdf5( name );
  }
  virtual EObjType::EEnum type() const { return m_common_fg.type(); }
  virtual string name() const { return m_common_fg.name(); }
  virtual obj_ptr_list children() const { return m_common_fg.children(); }
};

}}

#endif
