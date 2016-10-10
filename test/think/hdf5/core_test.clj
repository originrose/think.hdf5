(ns think.hdf5.core-test
  (:require [clojure.test :refer :all]
            [think.hdf5.core :refer :all]
            [think.resource.core :as resource]))


(deftest basic-test
  (resource/with-resource-context
    (let [test-file (open-file "test_data/h5ex_g_create.h5")
          test-data (mapv ->clj (get-children test-file))]
      (is (= test-data [{:attributes [] :children [] :name "G1" :type :group}])))))


(deftest reference-test
  (resource/with-resource-context
    (let [test-file (open-file "test_data/h5ex_t_objref.h5")
          test-data (->clj (first (get-children test-file)))
          refs (:data test-data)]
      (is (= (mapv get-name refs) ["/G1" "/DS2"])))))
