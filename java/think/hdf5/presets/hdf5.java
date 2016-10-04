package think.hdf5.presets;

import org.bytedeco.javacpp.annotation.*;
import org.bytedeco.javacpp.tools.*;

@Properties(target="think.hdf5.hdf5",
	    value={@Platform(include={"<hdf5.hpp>", "<hdf5_export.hpp>"},
			     includepath={"/usr/include/hdf5/serial", "hdf5pp/src"},
			     link="hdf5_cpp")})

public class hdf5 implements InfoMapper {
    public void map(InfoMap infoMap) {
    }
}

