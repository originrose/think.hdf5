(ns think.hdf5.core
  (:require [resource.core :as resource])
  (:import [think.hdf5 hdf5$library hdf5$Access hdf5$object hdf5$EObjType
            hdf5$dataset hdf5$attribute hdf5$abstract_ds]
           [org.bytedeco.javacpp BytePointer FloatPointer ShortPointer]))


(defn byte-ptr->string
  [^BytePointer byte-ary]
  (let [item-count (.capacity byte-ary)
        item-seq (map #(.get byte-ary %) (range item-count))]
    (String. ^"[B" (into-array Byte/TYPE (take-while #(not= % 0) item-seq)))))


(defn library-version
  []
  (let [majnum (int-array 1)
        minnum (int-array 1)
        relnum (int-array 1)]
    (hdf5$library/getLibVersion majnum minnum relnum)
    {:major (aget majnum 0)
     :minor (aget minnum 0)
     :release (aget relnum 0)}))

(defn init-library
  []
  (hdf5$library/initLibrary))

(defn close-library
  []
  (hdf5$library/termH5cpp))

(extend-protocol resource/PResource
  hdf5$object
  (release-resource [item] (hdf5$library/close_file item)))

(defn open-file
  ^hdf5$object [^String fname]
  ;;You only need to track the root returned by open file.  There is no
  ;;need to track anything else as the library will do that automagically.
  (resource/track (hdf5$library/open_file fname hdf5$Access/rdonly)))

(defn build-lookup
  [lookup-table-pairs]
  (let [forward-map (into {} lookup-table-pairs)]
   {:forward forward-map
    :backward (clojure.set/map-invert forward-map)}))

(def object-types
  (build-lookup [
                 [:unknown hdf5$EObjType/unknown]
                 [:group hdf5$EObjType/group]
                 [:dataset hdf5$EObjType/dataset]
                 [:datatype hdf5$EObjType/datatype]
                 [:file hdf5$EObjType/file]]))

(defn int->object-type [idx] (get (:backward object-types) idx))
(defn object-type->int [idx] (get (:forward object-types) idx))

(def data-types
  (build-lookup (mapv vec
                      (partition 2 [
                               :dt_integer hdf5$abstract_ds/dt_no_class
                               :dt_integer hdf5$abstract_ds/dt_integer
                               :dt_float hdf5$abstract_ds/dt_float
                               :dt_time hdf5$abstract_ds/dt_time
                               :dt_string hdf5$abstract_ds/dt_string
                               :dt_bitfield hdf5$abstract_ds/dt_bitfield
                               :dt_opaque hdf5$abstract_ds/dt_opaque
                               :dt_compound hdf5$abstract_ds/dt_compound
                               :dt_reference hdf5$abstract_ds/dt_reference
                               :dt_enum hdf5$abstract_ds/dt_enum
                               :dt_vlen hdf5$abstract_ds/dt_vlen
                               :dt_array hdf5$abstract_ds/dt_array
                               ]))))

(defn int->data-type [idx] (get (:backward data-types) idx))
(defn data-type->int [idx] (get (:forward data-types) idx))

(defn get-children
  [^hdf5$object obj]
  (mapv (fn [idx]
          (.get_child obj idx))
        (range (.child_count obj))))

(defn get-name
  [^hdf5$object obj]
  (byte-ptr->string (.name obj)))

(defn get-type
  [^hdf5$object obj]
  (int->object-type (.type obj)))

(defn get-attributes
  [^hdf5$object obj]
  (mapv (fn [idx]
          (.get_attribute obj idx))
        (range (.get_attribute-count obj))))

(defn attr->clj
  [^hdf5$attribute attr]
  {:name (byte-ptr->string (.name attr))
   :data-type (int->data-type (.get_type_class attr))
   :mem-size (.get_in_mem_data_size attr)})

(defmulti ->clj (fn [item] (get-type item)))

(defmethod ->clj :group
  [^hdf5$object grp]
  {:type :group
   :name (byte-ptr->string (.name grp))
   :attributes (mapv attr->clj (get-attributes grp))
   :children (mapv ->clj (get-children grp))})

(defmethod ->clj :dataset
  [^hdf5$object obj]
  (let [ds (.to_dataset obj)
        abs-ds (.asabstract_ds (.to_dataset obj))
        mem-size (.get_in_mem_data_size abs-ds)
        data-type (int->data-type (.get_type_class abs-ds))
        ret-data (cond
                   (= data-type :dt_float)
                   (let [num-floats (quot mem-size Float/BYTES)
                         storage (FloatPointer. num-floats)
                         float-ary (float-array num-floats)]
                     (.read ds storage (* num-floats Float/BYTES))
                     (.get storage float-ary)
                     float-ary)
                   (= data-type :dt_integer)
                   (let [num-shorts (quot mem-size Short/BYTES)
                         storage (ShortPointer. num-shorts)
                         short-ary (short-array num-shorts)]
                     (.read ds storage (* num-shorts Short/BYTES))
                     (.get storage short-ary)
                     short-ary))]

    {:type :dataset
     :name (byte-ptr->string (.name obj))
     :attributes (mapv attr->clj (get-attributes obj))
     :data-type data-type
     :mem-size mem-size
     :data ret-data}))
