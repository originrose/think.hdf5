#ifndef HDF5_IMPL_H
#define HDF5_IMPL_H

#include "hdf5.hpp"
#include <hdf5.h>
#include <stdexcept>
#include <string>

namespace think {

  int hdf5::to_hdf5_access( int access ) {
    int retval = 0;
    if ( access & Access::excl ) retval |= H5F_ACC_EXCL;
    if ( access & Access::trunc ) retval |= H5F_ACC_TRUNC;
    if ( access & Access::rdonly ) retval |= H5F_ACC_RDONLY;
    if ( access & Access::rdrw ) retval |= H5F_ACC_RDWR;
    if ( access & Access::debug ) retval |= H5F_ACC_DEBUG;
    if ( access & Access::create ) retval |= H5F_ACC_CREAT;
    return retval;
  }

  hdf5::herr_t hdf5::H5open(void) {
    return ::H5open();
  }
  hdf5::herr_t hdf5::H5close(void) {
    return ::H5close();
  }
  hdf5::herr_t hdf5::H5get_libversion(unsigned *majnum, unsigned *minnum, unsigned *relnum) {
    return ::H5get_libversion( majnum, minnum, relnum );
  }
  hdf5::htri_t hdf5::H5Fis_hdf5(const char *filename) {
    return ::H5Fis_hdf5( filename );
  }
  hdf5::hid_t hdf5::H5Fopen(const char *filename, unsigned flags) {
    return ::H5Fopen( filename, to_hdf5_access( flags ), H5P_DEFAULT );
  }
  hdf5::ssize_t hdf5::H5Fget_obj_count(hid_t file_id, FileObjType::EEnum types) {
    return ::H5Fget_obj_count(file_id, types);
  }
  hdf5::ssize_t hdf5::H5Fget_obj_ids(hid_t file_id, FileObjType::EEnum types, size_t max_objs, hid_t *obj_id_list) {
    return ::H5Fget_obj_ids(file_id, types, max_objs, obj_id_list);
  }
  hdf5::herr_t hdf5::H5Fclose(hid_t file_id) {
    return ::H5Fclose( file_id );
  }
  hdf5::ObjType::EEnum hdf5::H5Iget_type(hid_t id) {
    return static_cast<hdf5::ObjType::EEnum>( ::H5Iget_type(id) );
  }
  hdf5::ssize_t hdf5::H5Iget_name(hid_t id, char *name/*out*/, size_t size) {
    return ::H5Iget_name(id, name, size);
  }
  hdf5::ssize_t hdf5::get_num_children(hid_t loc_id) {
    ::H5G_info_t src_info;
    herr_t retval = ::H5Gget_info( loc_id, &src_info );
    if ( retval >= 0 ) {
      return src_info.nlinks;
    }
    else {
      throw std::runtime_error("get_num_children failed.");
    }
    return 0;
  }
  hdf5::hid_t hdf5::open_child(hid_t loc_id, hsize_t idx)
  {
    using namespace std;
    ssize_t name_len = H5Lget_name_by_idx(loc_id, ".", H5_INDEX_NAME, H5_ITER_INC, idx, NULL, 0, H5P_DEFAULT);
    if(name_len < 0)
      throw runtime_error( "H5Lget_name_by_idx failed" );

    // now, allocate C buffer to get the name
    string name;
    name.resize(name_len+1);
    char* name_C = &name[0];
    memset(name_C, 0, name_len+1); // clear buffer

    name_len = H5Lget_name_by_idx(loc_id, ".", H5_INDEX_NAME, H5_ITER_INC, idx, name_C, name_len+1, H5P_DEFAULT);

    if (name_len < 0)
      throw runtime_error( "H5Lget_name_by_idx failed" );

    H5O_info_t objinfo;

    // Use C API to get information of the object
    herr_t ret_value = H5Oget_info_by_name(loc_id, name_C, &objinfo, H5P_DEFAULT);

    // Throw exception if C API returns failure
    if (ret_value < 0)
      throw runtime_error("H5Oget_info_by_name failed");
    // Return a valid type or throw an exception for unknown type
    else
      switch (objinfo.type)
      {
      case H5O_TYPE_GROUP:
      {
	hid_t group_id = H5Gopen2(loc_id, name.c_str(), H5P_DEFAULT);
	// If the opening of the group failed, throw an exception
	if (group_id < 0)
	  throw runtime_error("H5Gopen2 failed");
	return group_id;
      }
      break;
      case H5O_TYPE_DATASET:
      {
	hid_t dataset_id = H5Dopen2(loc_id, name.c_str(), H5P_DEFAULT);
	// If the dataset's opening failed, throw an exception
	if(dataset_id < 0)
	  throw runtime_error("H5Dopen2 failed");
	return dataset_id;
      }
      default:
	throw runtime_error("Unknown type of object");
      }

    return -1;
  }
  hdf5::ssize_t hdf5::get_num_attrs(hid_t loc_id) {
    H5O_info_t oinfo;    /* Object info */

    if(H5Oget_info(loc_id, &oinfo) < 0)
      throw std::runtime_error("H5Oget_info failed");
    else
        return(static_cast<int>(oinfo.num_attrs));
  }
  hdf5::hid_t hdf5::open_attribute(hid_t loc_id, hsize_t idx) {
    return H5Aopen_by_idx(loc_id, ".", H5_INDEX_CRT_ORDER,
			  H5_ITER_INC, static_cast<hsize_t>(idx)
			  , H5P_DEFAULT, H5P_DEFAULT);
  }
  hdf5::ssize_t hdf5::get_attribute_name(hid_t attr, size_t buf_size, char* /*out*/ name)
  {
    return H5Aget_name(attr, buf_size, name);
  }

  //!!closes the type!!
  inline hdf5::ssize_t get_datatype_native_size(hdf5::hid_t mem_type_id)
  {
    using namespace std;
    // Get the data type's size by first getting its native type then getting
    // the native type's size.
    hid_t native_type = H5Tget_native_type(mem_type_id, H5T_DIR_DEFAULT);
    if (native_type < 0)
        throw runtime_error("H5Tget_native_type failed");

    size_t type_size = H5Tget_size(native_type);
    if (type_size == 0)
        throw runtime_error("H5Tget_size failed");

    // Close the native type and the datatype of this attribute.
    if (H5Tclose(native_type) < 0)
        throw runtime_error("H5Tclose(native_type) failed");

    if (H5Tclose(mem_type_id) < 0)
        throw runtime_error( "H5Tclose(mem_type_id) failed");
    return type_size;
  }

  //!!closes the space!!
  inline hdf5::ssize_t get_dataspace_extent_npoints(hdf5::hid_t dataspace_id)
  {
    using namespace std;
    ssize_t num_elements = H5Sget_simple_extent_npoints(dataspace_id);
    if (num_elements < 0)
        throw runtime_error("H5Sget_simple_extent_npoints failed");
    // Close the dataspace
    if (H5Sclose(dataspace_id) < 0)
        throw runtime_error("H5Sclose failed");
    return num_elements;
  }

  //closes the type and space!!
  inline hdf5::ssize_t get_in_mem_data_size( hdf5::hid_t type_id, hdf5::hid_t dataspace )
  {
    return get_datatype_native_size(type_id) * get_dataspace_extent_npoints(dataspace);
  }

  hdf5::ssize_t hdf5::get_attribute_in_mem_data_size( hid_t attr_id )
  {
    using namespace std;
    // Get the data type of this attribute
    hid_t mem_type_id = H5Aget_type(attr_id);
    if (mem_type_id < 0)
        throw runtime_error("H5Aget_type failed");
    hid_t space_id = H5Aget_space(attr_id);
    if (space_id < 0)
        throw runtime_error("H5Aget_space failed");
    return get_in_mem_data_size( mem_type_id, space_id );
  }

  hdf5::ssize_t hdf5::get_dataset_in_mem_data_size( hid_t ds_id )
  {
    using namespace std;
    // Get the data type of this dataset
    hid_t mem_type_id = H5Dget_type(ds_id);
    if (mem_type_id < 0)
        throw runtime_error("H5Dget_type failed");

    hid_t space_id = H5Dget_space(ds_id);
    if (space_id < 0)
        throw runtime_error("H5Dget_space failed");

    return get_in_mem_data_size( mem_type_id, space_id );
  }

  hdf5::hid_t hdf5::get_attribute_data_type(hid_t attr_id )
  {
    return H5Aget_type(attr_id);
  }
  hdf5::hid_t hdf5::get_dataset_data_type(hid_t ds_id )
  {
    return H5Dget_type(ds_id);
  }
  hdf5::TypeClass::EEnum hdf5::get_datatype_class( hid_t data_type_id )
  {
    return static_cast<hdf5::TypeClass::EEnum>( H5Tget_class(data_type_id) );
  }
  hdf5::ssize_t hdf5::get_datatype_size( hid_t type_id )
  {
    return H5Tget_size( type_id );
  }
  hdf5::ssize_t hdf5::is_variable_len_string( hid_t type_id )
  {
    return H5Tis_variable_str( type_id );
  }
  hdf5::hid_t hdf5::create_str_type()
  {
    return H5Tcopy( H5T_C_S1 );
  }
  hdf5::hid_t hdf5::create_variable_str_type()
  {
    hid_t retval = H5Tcopy( H5T_C_S1 );
    H5Tset_size( retval, H5T_VARIABLE );
    return retval;
  }
  hdf5::herr_t hdf5::close_type( hid_t data_type_id )
  {
    return H5Tclose( data_type_id );
  }
  hdf5::hid_t hdf5::get_dataset_dataspace( hid_t ds_id )
  {
    return H5Dget_space( ds_id );
  }
  int hdf5::get_dataspace_ndims(hid_t dataspace_id )
  {
    return H5Sget_simple_extent_ndims( dataspace_id );
  }
  int hdf5::get_dataspace_dims(hid_t dataspace_id, hsize_t *dims, hsize_t *maxdims)
  {
    return H5Sget_simple_extent_dims(dataspace_id, dims, maxdims);
  }

  hdf5::herr_t hdf5::read_attr_data(hid_t attr_id, hid_t datatype_id, void* buf)
  {
    return H5Aread(attr_id, datatype_id, buf);
  }
  hdf5::herr_t hdf5::read_dataset_data( hid_t ds_id, hid_t datatype_id, void* buf)
  {
    return H5Dread( ds_id, datatype_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, buf );
  }
  hdf5::herr_t hdf5::vlen_reclaim(hid_t datatype, hid_t dataspace, void* buf)
  {
    return H5Dvlen_reclaim( datatype, dataspace, H5P_DEFAULT, buf );
  }
}

#endif
