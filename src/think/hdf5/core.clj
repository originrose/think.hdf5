(ns think.hdf5.core
  (:require [think.hdf5.api :as api]
            [think.resource.core :as resource]))

;;Note that resources are managed the think.resource.

(defn- track-obj
  [^long obj-id]
  (resource/track (api/->HDF5Object obj-id))
  obj-id)

(defn open-file
  ^long [fname]
  (let [retval (api/open-file fname)]
    (when (< retval 0)
      (throw (ex-info "Failed to open file:" {:fname fname})))
    (track-obj retval)))

(defn get-children
  [^long obj-id]
  (->> (api/get-children obj-id)
       (mapv track-obj)))

(defn ->clj
  [^long obj-id]
  (let [retval (api/->clj obj-id)]
    (when (= (get retval :data-type) :H5I_REFERENCE)
      (mapv track-obj (get retval :data)))
    retval))

(defn get-name
  [^long obj-id]
  (api/get-name obj-id))

(defn get-type
  [^long obj-id]
  (api/get-obj-type obj-id))

(defn child-map
  [^long obj-id]
  (api/child-map obj-id))
