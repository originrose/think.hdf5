(ns think.hdf5.compile
  (:gen-class)
  (:import [org.bytedeco.javacpp.tools Builder]))


(defn build-java-stub
  []
  (Builder/main (into-array String ["think.hdf5.presets.hdf5" "-d" "java"])))


(defn build-jni-lib
  []
  (Builder/main (into-array String ["think.hdf5.hdf5" "-d"
                                    (str (System/getProperty "user.dir")
                                         "/java/native/linux/x86_64")
                                    "-nodelete" ;;When shit doesn't work this is very helpful
                                    "-Xcompiler"
                                    (str "-I" (System/getProperty "user.dir")
                                         "/hdf5pp/src")
                                    "-Xcompiler"
                                    "-std=c++11"
                                    ])))

(defn -main
  [& args]
  (let [arg-val (first args)
        command (if arg-val
                  (keyword arg-val)
                  :build-jni-java)]
    (condp = command
      :build-jni-java ;;step 1
      (build-java-stub)
      :build-jni
      (build-jni-lib))))
