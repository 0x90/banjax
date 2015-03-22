#A quick introduction to Banjax
# A Quick Introduction to Banjax #

Banjax is a library for low-level programming of IEEE 802.11 wireless network interfaces on the GNU/Linux operating system. The library is intended to simplify the writing of programs that manipulate IEEE 802.11 traffic. Uses for banjax include traffic analysis, monitoring of link quality and capacity, implementing network utilities and sniffers. A key feature of banjax is that network traffic can be transparently captured to file and these files used as input for off-line processing (e.g. to process the results of experiments).

Banjax is written in C++ and can be used directly by client programs that are written in C++ or Guile (a dialect of Scheme which is used as a scripting language in GNU/Linux systems). Support for Guile is implemented using the Simplified Wrapper and Interface Generator (SWIG) and support for other languages is also possible although not implemented at this time.

In the following sections we describe the design of Banjax using UML class diagrams. The diagrams reproduced here are informative but the limitations in reproducing them in printed format means that the reader is not expected to consult them in detail. Detailed investigation can be made of the model itself which is available in the source code repository and can be viewed/edited using the ArgoUML CASE tool.

## Organization ##
The core banjax library is divided into four namespaces: `editors`, `dot11`, `net` and `util`. These namespaces serve to divide the code into areas of responsibility and minimize the dependencies between loosely-related abstractions. Namespaces are organized into a hierarchy and these are shown in the UML package diagram in the figure below. There are four namespaces in total in a hierarchy where each depends only on the public interfaces of the packages below it.

![http://banjax.googlecode.com/files/packages.png](http://banjax.googlecode.com/files/packages.png)

Banjax is packaged as a library which maybe either a conventional static library, a shared library or a dynamic library depending on the user's development environment. To make use of the library a C++ client program uses the pre-processor's `\#include` directive to make use of the appropriate header files and can then make calls against the library abstractions. During installation the library and its associated header files are copied to the appropriate system directories so that client programs can compile and link against them.

## Banjax Design ##
The purpose of banjax is to provide useful abstractions for manipulating IEEE 802.11 network traffic. Abstraction is the process of hiding incidental or unnecessary features and exposing the essential operations in an easy-to-use manner. The core abstractions of banjax are these:

  * `wnic` which represents a wireless network interface and supports reading and writing of data to that interface. `wnic` hides the actual source or sink of data and may send/receive using a real WNIC device, a file of traffic captured by `tcpdump` or provide specialized `wnic` wrappers that perform pre- or post- processing of the traffic.
  * `buffer` a bounds-checked region of memory representing a frame received by a `wnic`. Bounds-checking ensures that access is only ever made to the frame body. In contrast, in C programs often can be made to overrun allocated memory which can lead to security compromises.
  * `buffer_fragment` is a specialized `buffer` class for making bounds-checked access to a _part_ of another `buffer`. Several `buffer_fragment` objects can share access to a single `buffer`.
  * `frame` classes are special _dissector_ classes that client programs use to access the buffer content in terms of its logical structure rather than integer offsets and ensures conversion between host and network byte-order is done when necessary.
  * `frame_editor` types perform specific modifications to specific frame types and can be chained together to form an editing processes.

A key design decision for banjax is the mitigation of memory allocation and referencing errors in client code by careful interface design. Banjax makes extensive use of reference-counted smart pointers to manage heap allocated storage and ensures that memory accesses to `buffer` contents are within bounds. Attempting to access memory outside of the legitimate range will cause an `out_of_range` exception to be raised.

## Detailed Design ##
### The `wnic` Interface ###
The figure below is a UML class diagram that shows the key abstraction - the `wnic` interface - and those closely-related classes which it makes use of. The key operations supported by `wnic` are reading and writing and this is done in terms of `buffer` objects.

![http://banjax.googlecode.com/files/classes_banjax.png](http://banjax.googlecode.com/files/classes_banjax.png)

The `wnic` interface hides the actual source and sink of the traffic it processes and so code written to use this interface can as easily be used with data files as with real WNICs. Client code calls `wnic::open` to obtain a concrete instance. Devices are opened by name but if a name ending in ".pcap" is passed then it interprets the name as the pathname of a file. Such capture files make use of the `tcpdump` format which means that programs such as `tcpdump` and WireShark can be used to capture and/or inspect the message traffic.

### The Standard `wnic` Implementations ###
For the most part banjax clients do not need to be aware of which
`wnic` implementaiton they are using because appropriate
implementations will be created automatically by
`wnic::open`. Alternatively, clients can provide their own
implementation of `wnic` or explicitly make use of the
banjax-supplied implementations of the `wnic` interface which
are shown in the following figure.

![http://banjax.googlecode.com/files/classes_wnic.png](http://banjax.googlecode.com/files/classes_wnic.png)

There are two major types of implementations:
  * Classes derived from `abstract_wnic`. These provide stand-alone implementations of the `wnic` interface and are the source and/or sink of frames.
  * Classes derived from `wnic_wrapper`. These allow one `wnic` implementation to perform pre- and post-processing of frames obtained from another `wnic`. The `wnic_read_logger` and `wnic_write_logger` implementations, for example, log input and output traffic to `tcpdump`-formatted files.

The most basic implementation of a standalone implementation is `dummy_wnic` which produces no input and consumes all output. This latter behaviour can be useful during testing because it acts as a bit-bucket and will consume all frames sent to it.  The `linux_wnic` and (now obsoleted) `madwifi_wnic` implementations encapsulate access to a physical device under the GNU/Linux operating system. The former handles the behaviour of standard Linux WNICs and the latter exists to support the unique behaviours of the \madwifi device driver. An `offline_wnic` uses a `tcpdump`-format file as its source of frames and this means that banjax can be used to process the results of experiments and other captures.

The other standard `wnic` implementation are wrapper classes. These can be applied to an existing wnic to add new behaviours. Banjax provides `wnic_read_logger` and `wnic_write_logger` and these wrappers ensure that data read or written from a specified `wnic` is also read/written to a `tcpdump`-format capture file. The `wnic::open` function will automatically create one or both of these wrappers if "+r", "+w" or "+rw" is appended to the device name. This makes it easy to debug programs written using banjax because any frames consumed or produced will be available in a capture file and this can be invaluable when debugging program code.

### `buffer` and Related Types ###
Frames are passed to and from `wnic` interface using the `buffer` class. This is a concrete class which which enables bounds-checked accesses to be made to a block of memory. Any attempt to read beyond the boundaries of a frame results in an `out_of_range` exception being thrown. The `buffer` contents be accessed in either in big-endian or little-endian formats. The figure below is a class diagram diagram that shows `buffer` and those classes which are closely associated with it.

![http://banjax.googlecode.com/files/classes_buffer.png](http://banjax.googlecode.com/files/classes_buffer.png)

An important quirk of the `buffer` class is that it _never_ includes the Frame Check-Sum (FCS) that is attached to an IEEE 802.11 MAC layer frame. This is because many WNICs generate the FCS in hardware and many device drivers strip this value out of received frames. Rather than have client code check on whether or not the FCS is present; banjax uses a normal form in which the FCS is never present in the buffer.

`buffer_fragment` is always associated with a `buffer` and shares an almost identical interface. The `buffer_fragment` is used to constrain access to just a _part_ of a `buffer`'s contents. This is useful when dissecting a frame's logical content and processing the higher-layer payloads. A `buffer_fragment` allows for safe access without risk of buffer over-runs and avoids the need for user code to manipulate offsets and compute where frames end. Several `buffer_fragment`s can make use of the same underlying `buffer`.

Another complication hidden by banjax is that of different datalink encapsulations. It is common practise in GNU/Linux for different datalink types to require or provide additional information when sending or receiving frames. This is achieved by encapsulating the MAC layer frame within a data-link specific container where the additional information is usually contained in the encapsulation header. The Linux kernel encapsulates received frames and removes the encapsulation prior to transmission. The `wnic` and `buffer` handle several different datalink encapsulations transparently and provide this additional information using the `buffer_info` objects. This allows code to be agnostic about the frame representation used to communicate with the Linux kernel and to focus on the MAC layer frame and the desired behaviours.

### `frame` and Other Dissectors ###
A MAC-layer frame when received represents a particular message. When processing frames received from a `wnic` it is easier and less error-prone to address a frame in terms of its logical content than using offsets within a buffer. This is the purpose of the `frame` and other _dissector_ classes. These classes are unrelated by inheritance but all perform roughly the same task - to expose the frame's logical structure and hide the details of how individual fields are stored.

The `frame` class is the most basic dissector. It allows access to the fields of the MAC layer header shared by all frame types. (ACK frames are a special case - they contain only a partial MAC header but are classified as control frames. Access to any of the ```missing'' fields will result in an ``out\_of\_range` exception being raised.). `frame` and its associated specializations are shown in the figure below.

![http://banjax.googlecode.com/files/classes_frame.png](http://banjax.googlecode.com/files/classes_frame.png)

### The `frame_editor` Interface and Implementations ###
The `frame_editor` combines the visitor and chain-of-responsibility design pattern from Gamma et al. [1](1.md) and allows client code to process specific `frame` types without resorting to switch-on-type constructs. A program that processes only data frames can instantiate the `data_frame_editor` and will not have to deal with any other types of frames.

![http://banjax.googlecode.com/files/classes_frame_editor.png](http://banjax.googlecode.com/files/classes_frame_editor.png)

## Further Information ##

### License ###
The Banjax project is a free software project in which the source code is available for users to download, modify and re-distribute under the terms of the GNU Public License version 3.0.