(defproject thinktopic/hdf5 "0.1.2"
  :description "Small jni wrapper to read hdf5 files."
  :url "http://example.com/FIXME"
  :license {:name "Eclipse Public License"
            :url "http://www.eclipse.org/legal/epl-v10.html"}
  :dependencies [[org.clojure/clojure "1.8.0"]
                 [org.bytedeco/javacpp "1.2.4"]
                 [thinktopic/resource "1.1.0"]]

  :java-source-paths ["java"]
  :native-path "java/native/"
  :aot [think.hdf5.compile]
  :main think.hdf5.compile)
