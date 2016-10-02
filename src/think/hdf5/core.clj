(ns think.hdf5.core
  (:import [think.hdf5 hdf5$H5Library]))


(defn library-version
  []
  (let [majnum (int-array 1)
        minnum (int-array 1)
        relnum (int-array 1)]
    (hdf5$H5Library/getLibVersion majnum minnum relnum)
    {:major (aget majnum 0)
     :minor (aget minnum 0)
     :release (aget relnum 0)}))

(defn init-library
  []
  (hdf5$H5Library/initH5cpp))

(defn close-library
  []
  (hdf5$H5Library/termH5cpp))
