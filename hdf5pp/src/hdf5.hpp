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

    struct GroupStoragetType {
      enum EEnum {
	H5G_STORAGE_TYPE_UNKNOWN = -1,	/* Unknown link storage type	*/
	H5G_STORAGE_TYPE_SYMBOL_TABLE,      /* Links in group are stored with a "symbol table" */
	/* (this is sometimes called "old-style" groups) */
	H5G_STORAGE_TYPE_COMPACT,		/* Links are stored in object header */
	H5G_STORAGE_TYPE_DENSE 		/* Links are stored in fractal heap & indexed with v2 B-tree */
      };
    };

    struct H5GInfo {
      GroupStorageType 	storage_type;	        /* Type of storage for links in group */
      hsize_t 	        nlinks;		        /* Number of links in group */
      int64_t           max_corder;             /* Current max. creation order value for group */
      hbool_t           mounted;                /* Whether group has a file mounted on it */
    };

    static herr_t H5open(void);
    static herr_t H5close(void);
    static herr_t H5get_libversion(unsigned *majnum, unsigned *minnum, unsigned *relnum);
    static htri_t H5Fis_hdf5(const char *filename);
    static int to_hdf5_access( int access );
    static hid_t H5Fopen(const char *filename, unsigned flags );
    static ssize_t H5Fget_obj_count(hid_t file_id, FileObjType::EEnum types);
    static ssize_t H5Fget_obj_ids(hid_t file_id, FileObjType::EEnum types, size_t max_objs, hid_t *obj_id_list);
    static herr_t H5Fclose(hid_t file_id);
    
    static ObjType::EEnum H5Iget_type(hid_t id);
    static hid_t H5Iget_file_id(hid_t id);
    static ssize_t H5Iget_name(hid_t id, char *name/*out*/, size_t size);

    static herr_t H5Gget_info(hid_t loc_id, H5GInfo *ginfo);
  };

}

#endif
