(ns think.hdf5.core-test
  (:require [clojure.test :refer :all]
            [think.hdf5.core :refer :all]
            [resource.core :as resource]))


(deftest basic-test
  (resource/with-resource-context
    (let [test-file (open-file "test_data/h5ex_g_create.h5")
          test-data (mapv ->clj (get-children test-file))]
      (is (= test-data [{:attributes [] :children [] :name "G1" :type :group}])))))
