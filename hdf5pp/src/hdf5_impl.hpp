#ifndef HDF5_IMPL_H
#define HDF5_IMPL_H

#include "hdf5.hpp"
#include <hdf5.h>

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
  hdf5::hid_t hdf5::H5Iget_file_id(hid_t id) {
    return ::H5Iget_file_id(id);
  }
  hdf5::ssize_t hdf5::H5Iget_name(hid_t id, char *name/*out*/, size_t size) {
    return ::H5Iget_name(id, name, size);
  }

}

#endif
