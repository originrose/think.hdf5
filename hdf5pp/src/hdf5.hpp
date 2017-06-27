#ifndef HDF5HPP
#define HDF5HPP
#include <cstdlib>
#include <cinttypes>

namespace think {

  struct hdf5 {
    typedef int herr_t;
    typedef int htri_t;
    typedef long hid_t;
    typedef size_t ssize_t;
    typedef long long unsigned hsize_t;
    typedef bool hbool_t;


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

    struct ObjType {
      enum EEnum {
	H5I_UNINIT		= (-2), /*uninitialized type			    */
	H5I_BADID		= (-1),	/*invalid Type				    */
	H5I_FILE            = 1,  	/*type ID for File objects      	    */
	H5I_GROUP,	                /*type ID for Group objects     	    */
	H5I_DATATYPE,	        /*type ID for Datatype objects		    */
	H5I_DATASPACE,	        /*type ID for Dataspace objects		    */
	H5I_DATASET,	        /*type ID for Dataset objects		    */
	H5I_ATTR,		        /*type ID for Attribute objects		    */
	H5I_REFERENCE,	        /*type ID for Reference objects		    */
	H5I_VFL,			/*type ID for virtual file layer	    */
	H5I_GENPROP_CLS,            /*type ID for generic property list classes */
	H5I_GENPROP_LST,            /*type ID for generic property lists        */
	H5I_ERROR_CLASS,            /*type ID for error classes		    */
	H5I_ERROR_MSG,              /*type ID for error messages		    */
	H5I_ERROR_STACK,            /*type ID for error stacks		    */
	H5I_NTYPES		        /*number of library types, MUST BE LAST!    */
      };
    };

    struct EDatasetType {
      enum EEnum {
	dt_no_class = -1,
	dt_integer = 0,
	dt_float = 1,
	dt_time = 2,
	dt_string = 3,
	dt_bitfield = 4,
	dt_opaque = 5,
	dt_compound = 6,
	dt_reference = 7,
	dt_enum = 8,
	dt_vlen = 9,
	dt_array = 10,
      };
    };

    struct FileObjType {
      enum EEnum {
	OBJ_FILE	 = 0x0001u,
	OBJ_DATASET	 = 0x0002u,
	OBJ_GROUP	 = 0x0004u,
	OBJ_DATATYPE     = 0x0008u,
	OBJ_ATTR         = 0x0010u,
	OBJ_ALL 	 = OBJ_FILE|OBJ_DATASET|OBJ_GROUP|OBJ_DATATYPE|OBJ_ATTR,
      };
    };

    struct GroupStorageType {
      enum EEnum {
	H5G_STORAGE_TYPE_UNKNOWN = -1,	/* Unknown link storage type	*/
	H5G_STORAGE_TYPE_SYMBOL_TABLE,      /* Links in group are stored with a "symbol table" */
	/* (this is sometimes called "old-style" groups) */
	H5G_STORAGE_TYPE_COMPACT,		/* Links are stored in object header */
	H5G_STORAGE_TYPE_DENSE 		/* Links are stored in fractal heap & indexed with v2 B-tree */
      };
    };

    struct H5GInfo {
      GroupStorageType::EEnum 	storage_type;	        /* Type of storage for links in group */
      size_t 	                nlinks;		        /* Number of links in group */
      long long                 max_corder;             /* Current max. creation order value for group */
      int                       mounted;                /* Whether group has a file mounted on it */
    };

    struct TypeClass
    {
      enum EEnum {
	H5T_NO_CLASS         = -1,  /*error                                      */
	H5T_INTEGER          = 0,   /*integer types                              */
	H5T_FLOAT            = 1,   /*floating-point types                       */
	H5T_TIME             = 2,   /*date and time types                        */
	H5T_STRING           = 3,   /*character string types                     */
	H5T_BITFIELD         = 4,   /*bit field types                            */
	H5T_OPAQUE           = 5,   /*opaque types                               */
	H5T_COMPOUND         = 6,   /*compound types                             */
	H5T_REFERENCE        = 7,   /*reference types                            */
	H5T_ENUM		 = 8,	/*enumeration types                          */
	H5T_VLEN		 = 9,	/*Variable-Length types                      */
	H5T_ARRAY	         = 10,	/*Array types                                */

	H5T_NCLASSES                /*this must be last                          */
      };
    };

    static herr_t H5open(void);
    static herr_t H5close(void);
    static herr_t H5get_libversion(unsigned *majnum, unsigned *minnum, unsigned *relnum);
    static htri_t H5Fis_hdf5(const char *filename);
    static int to_hdf5_access( int access );
    static hid_t H5Fopen(const char *filename, unsigned flags );
    static ssize_t H5Fget_obj_count(hid_t file_id, FileObjType::EEnum types);
    static ssize_t H5Fget_obj_ids(hid_t file_id, FileObjType::EEnum types, size_t max_objs, hid_t *obj_id_list);

    static ObjType::EEnum get_object_type(hid_t id);
    //Get the absolute name of an object
    static ssize_t get_name(hid_t id, char *name/*out*/, size_t size);
    //Close anything; file, object, group, dataspace, etc.
    static herr_t close_object( hid_t obj );

    static ssize_t get_num_children(hid_t loc_id);
    //Get the relative name of a child with respect to this object
    static ssize_t get_child_name( hid_t loc_id, ssize_t idx, char* name /*out*/, size_t size);
    //Some abstraction here to avoid lots of spurious arguments.
    static hid_t open_child(hid_t loc_id, hsize_t idx);

    static ssize_t get_num_attrs(hid_t loc_id);
    static hid_t open_attribute(hid_t loc_id, hsize_t idx);

    static hid_t open_datatype(hid_t attr_or_dataset_id );
    static TypeClass::EEnum get_datatype_class(hid_t type_id);
    static herr_t open_native_datatype( hid_t type_id );
    static ssize_t get_datatype_size( hid_t type_id );
    static ssize_t is_variable_len_string( hid_t type_id );
    static ssize_t get_datatype_native_size( hid_t type_id );
    static hid_t create_str_type();
    static hid_t create_variable_str_type();
    static herr_t set_datatype_size( hid_t dtype, size_t size );

    static hid_t open_dataspace(hid_t attr_or_dataset_id);
    static ssize_t get_dataspace_num_elements( hid_t ds_id );
    static int get_dataspace_ndims(hid_t dataspace_id );
    static int get_dataspace_dims(hid_t dataspace_id, hsize_t *dims, hsize_t *maxdims);

    static herr_t read_data(hid_t attr_or_dataset_id, hid_t datatype_id, void* buf);

    static hid_t dereference(hid_t src_obj, ssize_t file_offset);
  };

}

#endif
