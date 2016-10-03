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
  };

  class attribute_impl : public attribute
  {
    Attribute m_attribute;
  public:
    attribute_impl( const Attribute& att ) : m_attribute( att ){}
    virtual abstract_ds::EDType get_type_class() const {
      return static_cast<abstract_ds::EDType>( m_attribute.getTypeClass() );
    }
    virtual size_t get_in_mem_data_size() const {
      return m_attribute.getInMemDataSize();
    }
    virtual string name() const { return m_attribute.getName(); }
    virtual void read(void* buf, size_t buf_size) const {
      if ( buf_size >= get_in_mem_data_size() )
	m_attribute.read( m_attribute.getDataType(), buf );
    }
  };

  class common_loc
  {
    H5Location& m_location;
    mutable shared_attr_ptr_list m_attributes;
  public:
    common_loc( H5Location& inLocation ) : m_location( inLocation ) {}
    size_t get_attribute_count() const { return ensure_attributes().size(); }
    attribute* get_attribute( size_t idx ) { return ensure_attributes().at(idx).get(); }

  protected:
    virtual shared_attr_ptr_list& ensure_attributes() const
    {
      if ( m_attributes.empty() ) {
	size_t num_attrs = m_location.getNumAttrs();
	for ( size_t idx = 0; idx < num_attrs; ++idx ) {
	  m_attributes.push_back(
	    shared_attr_ptr( new attribute_impl( m_location.openAttribute( idx ) ) ) );
	}
      }
      return m_attributes;
    }
  };

  class common_fg : public common_loc
  {
    CommonFG& m_file_or_group;
    EObjType::EEnum m_type;
    mutable shared_obj_ptr_list m_children;
    string m_name;
  public:
    common_fg( CommonFG& file_or_group, H5Location& location,
	       EObjType::EEnum type, const string& name)
      : common_loc( location )
      , m_file_or_group( file_or_group )
      , m_type( type )
      , m_name( name )
    {
    }
    EObjType::EEnum type() const { return m_type; }
    string name() const { return m_name; }
    size_t child_count() const { return ensure_children().size(); }
    object* get_child( size_t idx ) { return ensure_children().at(idx).get(); }
  protected:
    shared_obj_ptr create_child( size_t idx ) const;
    virtual shared_obj_ptr_list& ensure_children() const
    {
      if ( m_children.empty() ) {
	size_t num_objs = m_file_or_group.getNumObjs();
	for ( size_t idx = 0; idx < num_objs; ++idx ) {
	  m_children.push_back(create_child(idx));
	}
      }
      return m_children;
    }
  };

  class group : public object
  {
    Group m_group;
    common_fg m_common_fg;
  public:
    group( const Group& group, const string& name )
      : m_group( group )
      , m_common_fg( m_group, m_group, EObjType::group, name )
    {
    }
    string name() const { return m_common_fg.name(); }
    EObjType::EEnum type() const { return EObjType::group; }
    size_t child_count() const { return m_common_fg.child_count(); }
    object* get_child( size_t idx ) { return m_common_fg.get_child( idx ); }
    size_t get_attribute_count() const { return m_common_fg.get_attribute_count(); }
    attribute* get_attribute( size_t idx ) { return m_common_fg.get_attribute (idx ); }
  };

  class dataset_impl : public dataset
  {
    DataSet m_dataset;
    common_loc m_location;
    string m_name;
  public:
    dataset_impl(const DataSet& inDataset, const string& inName)
      : m_dataset( inDataset )
      , m_location( m_dataset )
      , m_name( inName )
    {
    }
    string name() const { return m_name; }
    EObjType::EEnum type() const { return EObjType::dataset; }
    size_t get_attribute_count() const { return m_location.get_attribute_count(); }
    attribute* get_attribute( size_t idx ) { return m_location.get_attribute(idx); }
    EDType get_type_class() const {
      return static_cast<abstract_ds::EDType>( m_dataset.getTypeClass() ); }
    size_t get_in_mem_data_size() const { return m_dataset.getInMemDataSize(); }
    void read( void* buf, size_t buf_size ) const
    {
      if ( buf_size >= get_in_mem_data_size() )
	m_dataset.read( buf, m_dataset.getDataType() );
    }
    dataset* to_dataset() { return this; }
  };

  shared_obj_ptr common_fg::create_child( size_t idx ) const
  {
    string child_name = m_file_or_group.getObjnameByIdx( idx );
    int child_type = m_file_or_group.childObjType( idx );
    shared_obj_ptr new_obj;
    if ( child_type == H5O_TYPE_GROUP )
      new_obj = shared_obj_ptr( new group( m_file_or_group.openGroup( child_name ),
					   child_name ) );
    else if ( child_type == H5O_TYPE_DATASET )
      new_obj = shared_obj_ptr( new dataset_impl( m_file_or_group.openDataSet( child_name ),
						  child_name ) );
    else
      new_obj = shared_obj_ptr ( new rand_obj( child_name,
					       static_cast<EObjType::EEnum>( child_type ) ) );
    return new_obj;
  }


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
    , m_common_fg( m_file, m_file, EObjType::file, name )
  {
  }
  static bool is_hdf5_file( const char* name )
  {
    return H5File::isHdf5( name );
  }
  EObjType::EEnum type() const { return m_common_fg.type(); }
  string name() const { return m_common_fg.name(); }
  size_t child_count() const { return m_common_fg.child_count(); }
  object* get_child( size_t idx ) { return m_common_fg.get_child( idx ); }
  size_t get_attribute_count() const { return m_common_fg.get_attribute_count(); }
  attribute* get_attribute( size_t idx ) { return m_common_fg.get_attribute (idx ); }
};

}}

#endif
