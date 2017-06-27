(defproject thinktopic/hdf5 "0.2.2-SNAPSHOT"
  :description "Small jni wrapper to read hdf5 files."
  :url "http://example.com/FIXME"
  :license {:name "Eclipse Public License"
            :url "http://www.eclipse.org/legal/epl-v10.html"}
  :dependencies [[org.clojure/clojure "1.8.0"]
                 [org.bytedeco/javacpp "1.2.4"]
                 [thinktopic/think.resource "1.2.1"]]

  :java-source-paths ["java"]
  :native-path "java/native/"
  :aot [think.hdf5.compile]
  :main think.hdf5.compile)
