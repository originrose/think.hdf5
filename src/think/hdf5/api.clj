(ns think.hdf5.api
  (:require [think.resource.core :as resource]
            [clojure.set :as set])
  (:import [think.hdf5 api$hdf5$Access api$hdf5 api$hdf5$FileObjType
            api$hdf5$ObjType api$hdf5$TypeClass]
           [org.bytedeco.javacpp BytePointer FloatPointer ShortPointer
            LongPointer IntPointer DoublePointer Pointer CLongPointer]
           [java.lang.reflect Field]))

(defn- get-private-field [^Class cls field-name]
  (let [^Field field (first (filter
                             (fn [^Field x] (.. x getName (equals field-name)))
                             (.getDeclaredFields cls)))]
    (.setAccessible field true)
    field))

(defonce address-field (get-private-field Pointer "address"))
(defonce position-field (get-private-field Pointer "position"))
(defonce limit-field (get-private-field Pointer "limit"))
(defonce capacity-field (get-private-field Pointer "capacity"))

(defn unsafe-read-byte
  [^BytePointer byte-ary ^long idx]
  (.set ^Field capacity-field byte-ary (inc idx))
  (.set ^Field limit-field byte-ary (inc idx))
  (let [retval (.get byte-ary idx)]
    retval))

(defn variable-byte-ptr->string
  [^BytePointer byte-ary]
  (String. ^"[B"
           (into-array Byte/TYPE
                       (take-while #(not= % 0)
                                   (map #(unsafe-read-byte
                                          byte-ary %)
                                        (range))))))

(defn construct-byte-ptr
  [addr len]
  (let [retval (BytePointer.)]
    (.set ^Field address-field retval addr)
    (.set ^Field limit-field retval len)
    (.set ^Field capacity-field retval len)
    retval))

(defn byte-ptr->string
  [^BytePointer byte-ary]
  (let [item-count (.capacity byte-ary)
        item-seq (map #(.get byte-ary %) (range item-count))]
    (String. ^"[B" (into-array Byte/TYPE (take-while #(not= % 0) item-seq)))))

(defprotocol PPtrToArray
  (->array [ptr]))

(extend-protocol PPtrToArray
  DoublePointer
  (->array [^DoublePointer ptr]
    (let [retval (double-array (.capacity ptr))]
      (.get ptr retval)
      retval))
  FloatPointer
  (->array [^FloatPointer ptr]
    (let [retval (float-array (.capacity ptr))]
      (.get ptr retval)
      retval))
  LongPointer
  (->array [^LongPointer ptr]
    (let [retval (long-array (.capacity ptr))]
      (.get ptr retval)
      retval))
  IntPointer
  (->array [^IntPointer ptr]
    (let [retval (int-array (.capacity ptr))]
      (.get ptr retval)
      retval))
  ShortPointer
  (->array [^ShortPointer ptr]
    (let [retval (short-array (.capacity ptr))]
      (.get ptr retval)
      retval))
  BytePointer
  (->array [^BytePointer ptr]
    (let [retval (byte-array (.capacity ptr))]
      (.get ptr retval)
      retval)))

(defn variable-len-str->string-vec
  [^LongPointer long-ary]
  (->> (vec (->array long-ary))
       (mapv (fn [^long ptr-addr]
               (let [^BytePointer str-ptr (construct-byte-ptr ptr-addr 0)]
                 (variable-byte-ptr->string str-ptr))))))

(defn library-version
  []
  (let [majnum (int-array 1)
        minnum (int-array 1)
        relnum (int-array 1)]
    (api$hdf5/H5get_libversion majnum minnum relnum)
    {:major (aget majnum 0)
     :minor (aget minnum 0)
     :release (aget relnum 0)}))

(defn init-library
  "Returns non-negative on success"
  []
  (api$hdf5/H5open))

(defn close-library
  []
  (api$hdf5/H5close))

(defn open-file
  ^long [^String fname]
  ;;You only need to track the root returned by open file.  There is no
  ;;need to track anything else as the library will do that automagically.
  (api$hdf5/H5Fopen fname api$hdf5$Access/rdonly))

(defn close-object
  ^long [^long obj-id]
  (api$hdf5/close-object obj-id))

(defrecord HDF5Object [^long obj-handle]
  resource/PResource
  (release-resource [this] (close-object obj-handle)))

(defn get-file-obj-count
  ^long [^long file-id]
  (api$hdf5/H5Fget_obj_count file-id api$hdf5$FileObjType/OBJ_ALL))

(defn get-file-objects
  [^long file-id]
  (let [num-objects (get-file-obj-count file-id)
        retval (CLongPointer. num-objects)
        num-received (api$hdf5/H5Fget_obj_ids file-id api$hdf5$FileObjType/OBJ_ALL
                                              num-objects retval)]
    (->> (range num-received)
         (map #(.get retval (long %)))
         vec)))

(def objtype-names-map
  {api$hdf5$ObjType/H5I_UNINIT :H5I_UNINIT
   api$hdf5$ObjType/H5I_BADID :H5I_BADID
   api$hdf5$ObjType/H5I_FILE :H5I_FILE
   api$hdf5$ObjType/H5I_GROUP :H5I_GROUP
   api$hdf5$ObjType/H5I_DATATYPE :H5I_DATATYPE
   api$hdf5$ObjType/H5I_DATASPACE :H5I_DATASPACE
   api$hdf5$ObjType/H5I_DATASET :H5I_DATASET
   api$hdf5$ObjType/H5I_ATTR :H5I_ATTR
   api$hdf5$ObjType/H5I_REFERENCE :H5I_REFERENCE
   api$hdf5$ObjType/H5I_VFL :H5I_VFL
   api$hdf5$ObjType/H5I_GENPROP_CLS :H5I_GENPROP_CLS
   api$hdf5$ObjType/H5I_GENPROP_LST :H5I_GENPROP_LST
   api$hdf5$ObjType/H5I_ERROR_CLASS :H5I_ERROR_CLASS
   api$hdf5$ObjType/H5I_ERROR_MSG :H5I_ERROR_MSG
   api$hdf5$ObjType/H5I_ERROR_STACK :H5I_ERROR_STACK
   api$hdf5$ObjType/H5I_NTYPES :H5I_NTYPES})

(def objtype-kwd-map (set/map-invert objtype-names-map))

(defn objtype->kw
  [^long obj-type]
  (get objtype-names-map obj-type))

(defn kw->objtype
  [obj-kw]
  (get objtype-kwd-map obj-kw))

(defn get-obj-type
  [^long obj-id]
  (-> (api$hdf5/get_object_type obj-id)
      objtype->kw))

(defn get-obj-name
  [^long obj-id]
  (let [name-len (api$hdf5/get_name obj-id (BytePointer. 0) 0)
        retval (BytePointer. (+ name-len 1))]
    (api$hdf5/get_name obj-id retval (+ name-len 1))
    (byte-ptr->string retval)))

(defn get-num-children
  [^long obj-id]
  (api$hdf5/get_num_children obj-id))

(defn open-child
  ^long [^long obj-id ^long idx]
  (api$hdf5/open_child obj-id idx))

(defn get-num-attributes
  ^long [^long obj-id]
  (api$hdf5/get_num_attrs obj-id))

(defn open-attribute
  ^long [^long obj-id ^long idx]
  (api$hdf5/open_attribute obj-id idx))

(defn open-datatype
  ^long [^long attr-or-dataset-id]
  (api$hdf5/open_datatype attr-or-dataset-id))

(def typeclass-kwd-map
  {api$hdf5$TypeClass/H5T_NO_CLASS :H5T_NO_CLASS
   api$hdf5$TypeClass/H5T_INTEGER :H5T_INTEGER
   api$hdf5$TypeClass/H5T_FLOAT :H5T_FLOAT
   api$hdf5$TypeClass/H5T_TIME :H5T_TIME
   api$hdf5$TypeClass/H5T_STRING :H5T_STRING
   api$hdf5$TypeClass/H5T_BITFIELD :H5T_BITFIELD
   api$hdf5$TypeClass/H5T_OPAQUE :H5T_OPAQUE
   api$hdf5$TypeClass/H5T_COMPOUND :H5T_COMPOUND
   api$hdf5$TypeClass/H5T_REFERENCE :H5T_REFERENCE
   api$hdf5$TypeClass/H5T_ENUM :H5T_ENUM
   api$hdf5$TypeClass/H5T_VLEN :H5T_VLEN
   api$hdf5$TypeClass/H5T_ARRAY :H5T_ARRAY
   api$hdf5$TypeClass/H5T_NCLASSES :H5T_NCLASSES})

(def kwd-typeclass-map (set/map-invert typeclass-kwd-map))

(defn get-datatype-class
  [^long datatype]
  (get typeclass-kwd-map (api$hdf5/get_datatype-class datatype)))

(defn get-datatype-size
  ^long [^long datatype]
  (api$hdf5/get_datatype_size datatype))

(defn get-in-mem-size
  ^long [^long attr-or-dataset-id]
  (let [dtype (api$hdf5/open_datatype attr-or-dataset-id)
        type-size (api$hdf5/get_datatype_native_size dtype)
        dspace (api$hdf5/open_dataspace attr-or-dataset-id)
        num-elems (api$hdf5/get_dataspace_num_elements dspace)]
    (close-object dtype)
    (close-object dspace)
    (* type-size num-elems)))

(defn is-variable-len-string?
  [^long datatype]
  (> (api$hdf5/is_variable_len_string datatype) 0))

(defn create-str-datatype
  ^long []
  (api$hdf5/create_str_type))

(defn create-variable-len-str-type
  ^long []
  (api$hdf5/create_variable_str_type))

(defn open-dataspace
  ^long [^long attr-or-dataset-id]
  (api$hdf5/open-dataspace attr-or-dataset-id))

(defn get-dataspace-num-elements
  [^long dataspace]
  (api$hdf5/get_dataspace_num_elements dataspace))

(defn get-dataspace-ndims
  ^long [^long dataspace-id]
  (api$hdf5/get_dataspace_ndims dataspace-id))

(defn get-dataspace-dims
  [^long dataspace-id]
  (let [n-dims (get-dataspace-ndims dataspace-id)
        dims (LongPointer. n-dims)
        maxdims (LongPointer. n-dims)]
    (api$hdf5/get_dataspace_dims dataspace-id dims maxdims)
    {:dims (vec (->array dims))
     :max-dims (vec (->array maxdims))}))

(defn read-data
  [^long attr-or-dataset-id ^long datatype-id ^Pointer buf]
  (api$hdf5/read_data attr-or-dataset-id datatype-id buf))

(defn get-children
  [^long obj]
  (let [obj-type (get-obj-type obj)]
    (when (or (= obj-type :H5I_FILE)
              (= obj-type :H5I_GROUP))
      (mapv (partial open-child obj)
            (range (get-num-children obj))))))

(defn get-name
  ^String [^long obj]
  (get-obj-name obj))

(defn get-type
  [^long obj]
  (get-obj-type obj))

(defn child-map
  "Given a node return a map of keyword-name to child-node"
  [node]
  (into {} (map (fn [node-child]
                  [(keyword (get-name node-child))
                   node-child])
                (get-children node))))

(defn get-attributes
  [^long obj]
  (mapv (fn [idx]
          (open-attribute obj idx))
        (range (get-num-attributes obj))))

(defn attr->clj
  [^long attr]
  (let [datatype-obj (open-datatype attr)
        data-type (get-datatype-class datatype-obj)
        mem-size (get-in-mem-size attr)
        data-buffer (when (= data-type :H5T_STRING)
                      (let [byte-ary (BytePointer. mem-size)]
                        (read-data attr datatype-obj byte-ary)
                        (byte-ptr->string byte-ary)))]
    (close-object datatype-obj)
   {:name (get-name attr)
    :data-type data-type
    :mem-size mem-size
    :data data-buffer}))

(defmulti ->clj (fn [item] (get-type item)))

(defmethod ->clj :H5I_GROUP
  [^long group]
  {:type :group
   :name (get-obj-name group)
   :attributes (mapv attr->clj (get-attributes group))
   :children (mapv ->clj (get-children group))})

(defn decode-variable-len-string
  [^long dataset]
  (let [datatype-obj (open-datatype dataset)
        dataspace (open-dataspace dataset)
        {:keys [dims maxdims]} (get-dataspace-dims dataspace)
        num-longs (apply * dims)
        data-ptr (LongPointer. num-longs)]
    (read-data dataset datatype-obj data-ptr)
    (let [retval (variable-len-str->string-vec data-ptr)]
      (close-object datatype-obj)
      (close-object dataspace)
      retval)))

(defn decode-string
  [^long dataset]
  (let [datatype-obj (open-datatype dataset)
        dataspace-id (open-dataspace dataset)
        {:keys [dims]} (get-dataspace-dims dataspace-id)
        str-column-size (+ 1 (get-datatype-size datatype-obj))
        n-elems (apply * dims)
        string-size (* str-column-size n-elems)
        str-data (BytePointer. string-size)
        read-dtype (api$hdf5/create_str_type)]
    (api$hdf5/set_datatype_size read-dtype str-column-size)
    (read-data dataset read-dtype str-data)
    (let [retval (->> (range (quot string-size str-column-size))
                      (mapv (fn [idx]
                              (let [offset (* idx str-column-size)]
                                (-> (construct-byte-ptr (+ (.address str-data) offset) str-column-size)
                                    (variable-byte-ptr->string))))))]
      (close-object read-dtype)
      (close-object datatype-obj)
      (close-object dataspace-id)
      retval)))

(defn- numeric-type->array
  [dataset datatype-obj storage]
  (read-data dataset datatype-obj storage)
  (->array storage))


(defmethod ->clj :H5I_DATASET
  [^long dataset]
  (let [mem-size (get-in-mem-size dataset)
        datatype-obj (open-datatype dataset)
        data-type (get-datatype-class datatype-obj)
        dataspace (open-dataspace dataset)
        {:keys [dims max-dims]} (get-dataspace-dims dataspace)
        n-elems (reduce * dims)
        dtype-size (quot mem-size n-elems)
        retval     {:type :dataset
                    :name (get-name dataset)
                    :attributes (mapv attr->clj (get-attributes dataset))
                    :data-type data-type
                    :mem-size mem-size
                    :dimensions dims}]
    (assoc retval :data
           (condp = data-type
             :H5T_STRING
             (if (is-variable-len-string? datatype-obj)
               (decode-variable-len-string dataset)
               (decode-string dataset))
             :H5T_FLOAT
             (numeric-type->array dataset datatype-obj
                                  (if (= dtype-size 8)
                                    (DoublePointer. n-elems)
                                    (FloatPointer. n-elems)))
             :H5T_INTEGER
             (numeric-type->array dataset datatype-obj
                                  (condp = dtype-size
                                    8 (LongPointer. n-elems)
                                    4 (IntPointer. n-elems)
                                    2 (ShortPointer. n-elems)
                                    1 (BytePointer. n-elems)))
             :H5T_REFERENCE
             (let [storage (condp = dtype-size
                             8 (LongPointer. n-elems)
                             4 (IntPointer. n-elems)
                             2 (ShortPointer. n-elems)
                             1 (BytePointer. n-elems))
                   _ (read-data dataset datatype-obj storage)]
               ;;For references the reference is a file offset from the dataset where the object lies...
               (->> (->array storage)
                    (mapv (fn [^long file-offset]
                            (api$hdf5/dereference dataset file-offset)))))))))


(comment
  (def thefile (open-file "test_data/h5ex_t_string.h5"))
  (def first-child (open-child thefile 0))

  )
