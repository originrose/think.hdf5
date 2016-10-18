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


(deftest fixed-string-test
  (resource/with-resource-context
    (let [clj-data (->clj
                    (first (get-children (open-file "test_data/h5ex_t_string.h5"))))]
      (is (= (:data clj-data)
             ["Parting" "is such" "sweet" "sorrow."])))))


(deftest variable-string-test
  (resource/with-resource-context
    (let [clj-data (->clj
                    (first (get-children (open-file "test_data/strings.h5"))))]
      (is (= (:data clj-data)
             ["A fight is a contract that takes two people to honor."
              "A combative stance means that you've accepted the contract."
              "In which case, you deserve what you get."
              "  --  Professor Cheng Man-ch'ing"])))))


(defn test-mnist
  []
  (resource/with-resource-context
   (let [test-file (open-file "test_data/decomposed_mnist_model.h5")
         model-json (second (get-children test-file))]
     (->clj model-json))))
