(ns think.hdf5.core-test
  (:require [clojure.test :refer :all]
            [think.hdf5.core :as hdf5]
            [think.resource.core :as resource]))


(deftest basic-test
  (resource/with-resource-context
    (let [test-file (hdf5/open-file "test_data/h5ex_g_create.h5")
          test-data (mapv hdf5/->clj (hdf5/get-children test-file))]
      (is (= test-data [{:attributes [] :children [] :name "/G1" :type :group}])))))


(deftest reference-test
  (resource/with-resource-context
    (let [test-file (hdf5/open-file "test_data/h5ex_t_objref.h5")
          test-data (hdf5/->clj (first (hdf5/get-children test-file)))
          refs (:data test-data)]
      (is (= (mapv hdf5/get-name refs) ["/G1" "/DS2"])))))


(deftest fixed-string-test
  (resource/with-resource-context
    (let [clj-data (hdf5/->clj
                    (first (hdf5/get-children (hdf5/open-file "test_data/h5ex_t_string.h5"))))]
      (is (= (:data clj-data)
             ["Parting" "is such" "sweet" "sorrow."])))))


(deftest variable-string-test
  (resource/with-resource-context
    (let [clj-data (hdf5/->clj
                    (first (hdf5/get-children (hdf5/open-file "test_data/strings.h5"))))]
      (is (= (:data clj-data)
             ["A fight is a contract that takes two people to honor."
              "A combative stance means that you've accepted the contract."
              "In which case, you deserve what you get."
              "  --  Professor Cheng Man-ch'ing"])))))


(defn test-mnist
  []
  (resource/with-resource-context
   (let [test-file (hdf5/open-file "test_data/mnist.h5")
         model-json (second (hdf5/get-children test-file))]
     (hdf5/->clj model-json))))
