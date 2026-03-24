// EXAMPLE!!! FILE OF REAL STANDARD LIBRARY STREAM.

abstract struct Stream<T: Any> {
        work Write(data: T)
        work Read(): T
}

struct StdOutStream: Stream<String> {
        override work Write(data: String) {
                WriteNative -> data
        }
        override work Read(): String {
                 return -> ReadNative
        }

        // Native functions
        native work NativeWrite(data: String)
        native work NativeRead(): String
}

const StdOutStream stdout = StdOutStream()