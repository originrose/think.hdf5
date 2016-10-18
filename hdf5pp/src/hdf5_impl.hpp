#ifndef HDF5_IMPL_H
#define HDF5_IMPL_H

#include "hdf5.hpp"
#include <H5Fpublic.h>

namespace think { namespace hdf5 {

  class rand_obj : public object
  {
    object_registry& m_registry;
    string m_name;
    shared_obj_ptr_list m_children;
    EObjType::EEnum m_type;

  public:
    rand_obj( object_registry& reg, string& name, EObjType::EEnum type )
      : m_registry( reg )
      , m_name (name )
      , m_type( type )
    {
    }
    virtual EObjType::EEnum type() const { return m_type; }
    virtual string name() const { return m_name; }
    virtual object_registry& registry() { return m_registry; }
    int obj_id () const { return 0; }
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
    object_registry& m_registry;
    CommonFG& m_file_or_group;
    EObjType::EEnum m_type;
    mutable shared_obj_ptr_list m_children;
    string m_name;
  public:
    common_fg( object_registry& registry,
	       CommonFG& file_or_group, H5Location& location,
	       EObjType::EEnum type, const string& name)
      : common_loc( location )
      , m_registry( registry )
      , m_file_or_group( file_or_group )
      , m_type( type )
      , m_name( name )
    {
    }
    EObjType::EEnum type() const { return m_type; }
    string name() const { return m_name; }
    size_t child_count() const { return ensure_children().size(); }
    object* get_child( size_t idx ) { return ensure_children().at(idx).get(); }
    object_registry& registry() { return m_registry; }
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
    group( object_registry& reg, const Group& group, const string& name )
      : m_group( group )
      , m_common_fg( reg, m_group, m_group, EObjType::group, name )
    {
    }
    string name() const { return m_common_fg.name(); }
    EObjType::EEnum type() const { return EObjType::group; }
    int obj_id () const { return m_group.getId(); }
    size_t child_count() const { return m_common_fg.child_count(); }
    object* get_child( size_t idx ) { return m_common_fg.get_child( idx ); }
    size_t get_attribute_count() const { return m_common_fg.get_attribute_count(); }
    attribute* get_attribute( size_t idx ) { return m_common_fg.get_attribute (idx ); }
    object_registry& registry() { return m_common_fg.registry(); }
  };

  class dataset_impl : public dataset
  {
    object_registry& m_registry;
    DataSet m_dataset;
    common_loc m_location;
    string m_name;
    hid_t m_memtype; //for variable length strings
  public:
    dataset_impl(object_registry& reg, const DataSet& inDataset, const string& inName)
      : m_registry( reg )
      , m_dataset( inDataset )
      , m_location( m_dataset )
      , m_name( inName )
      , m_memtype( 0 )
    {
    }
    string name() const { return m_name; }
    EObjType::EEnum type() const { return EObjType::dataset; }
    object_registry& registry() { return m_registry; }
    int obj_id () const { return m_dataset.getId(); }
    size_t get_attribute_count() const { return m_location.get_attribute_count(); }
    attribute* get_attribute( size_t idx ) { return m_location.get_attribute(idx); }
    EDType get_type_class() const {
      return static_cast<abstract_ds::EDType>( m_dataset.getTypeClass() ); }
    size_t get_in_mem_data_size() const { return m_dataset.getInMemDataSize(); }
    bool is_simple() const { return m_dataset.getSpace().isSimple(); }
    size_t ndims() const { return m_dataset.getSpace().getSimpleExtentNdims(); }
    void get_dims(void* buf) const
    {
      m_dataset.getSpace().getSimpleExtentDims( reinterpret_cast<hsize_t*>( buf ) );
    }
    void read( void* buf, size_t buf_size ) const
    {
      if ( buf_size >= get_in_mem_data_size() )
	m_dataset.read( buf, m_dataset.getDataType() );
    }
    dataset* to_dataset() { return this; }
    bool is_variable_len_string() const
    {
      hid_t filetype = H5Dget_type(obj_id());
      return H5Tis_variable_str(filetype) != 0;
    }
    size_t string_column_size() const
    {
      hid_t filetype = H5Dget_type(obj_id());
      size_t sdim = H5Tget_size(filetype);
      //make room for null terminator.
      ++sdim;
      return sdim;
    }
    size_t string_size() const
    {
      size_t sdim = string_column_size();
      if (ndims()) {
	hsize_t dims[10] = { 0 };
	if ( ndims() > 10 ) { throw std::exception(); }
	m_dataset.getSpace().getSimpleExtentDims( dims );
	hsize_t total_dims = 1;
	for ( size_t idx = 0; idx < ndims(); ++idx )
	  total_dims *= dims[idx];
	return static_cast<size_t>( sdim * total_dims );
      }
      else
	return sdim;
    }
    void read_variable_string(void* buf, size_t buf_size )
    {
      m_memtype = H5Tcopy(H5T_C_S1);
      H5Tset_size(m_memtype, H5T_VARIABLE);
      H5Dread(obj_id(), m_memtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, buf);
    }

    void release_variable_string(void* buf)
    {
      if ( m_memtype ) {
	H5Dvlen_reclaim( m_memtype, H5Dget_space( obj_id() ), H5P_DEFAULT, buf );
	H5Tclose( m_memtype );
	m_memtype = 0;
      }
    }

    void read_string(void* buf)
    {
      hid_t memtype = H5Tcopy(H5T_C_S1);
      H5Tset_size( memtype, string_column_size() );
      H5Dread( obj_id(), memtype, H5S_ALL, H5S_ALL, H5P_DEFAULT, buf );
      H5Tclose( memtype );
    }
  };

  shared_obj_ptr common_fg::create_child( size_t idx ) const
  {
    string child_name = m_file_or_group.getObjnameByIdx( idx );
    int child_type = m_file_or_group.childObjType( idx );
    shared_obj_ptr new_obj;
    if ( child_type == H5O_TYPE_GROUP )
      new_obj = shared_obj_ptr( new group( m_registry,
					   m_file_or_group.openGroup( child_name ),
					   child_name ) );
    else if ( child_type == H5O_TYPE_DATASET )
      new_obj = shared_obj_ptr( new dataset_impl( m_registry,
						  m_file_or_group.openDataSet( child_name ),
						  child_name ) );
    else
      new_obj = shared_obj_ptr ( new rand_obj( m_registry,
					       child_name,
					       static_cast<EObjType::EEnum>( child_type ) ) );
    return new_obj;
  }


class file : public object, public object_registry
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
  typedef map<long,shared_obj_ptr> TRefMap;
  TRefMap m_referenced_objects;
public:
  file( const char* name, unsigned int flags )
    : m_file( name, to_hdf5_access( flags ) )
    , m_common_fg( *this, m_file, m_file, EObjType::file, name )
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
  object_registry& registry() { return *this; }
  int obj_id () const { return m_file.getId(); }
  object* dereference( int src_id, long file_offset )
  {
    TRefMap::iterator iter = m_referenced_objects.find( file_offset );
    if ( iter != m_referenced_objects.end() )
      return iter->second.get();

    void* ref = &file_offset;
    H5O_type_t obj_type = H5O_TYPE_UNKNOWN;
    H5Rget_obj_type( src_id, H5R_OBJECT, ref, &obj_type );
    object* retval = NULL;
    string name_str;
    hid_t obj_id = H5Rdereference( src_id, H5R_OBJECT, ref );
    if ( obj_id ) {
      size_t name_size = H5Iget_name( obj_id, NULL, 0 );
      name_str.resize( name_size + 1 );
      H5Iget_name( obj_id, (char*)name_str.c_str(), name_str.size() );
      switch( obj_type ) {
      case H5O_TYPE_GROUP:
	retval = new group( *this, Group(obj_id), name_str );
	break;
      case H5O_TYPE_DATASET:
	retval = new dataset_impl( *this, DataSet(obj_id), name_str );
	break;
      default:
	retval = new rand_obj( *this, name_str, static_cast<EObjType::EEnum>( obj_type ) );
	break;
      }
      shared_obj_ptr retval_ptr = shared_obj_ptr( retval );
      m_referenced_objects.insert( make_pair( file_offset, retval_ptr ) );
    }
    return retval;
  }
};

}}

#endif
