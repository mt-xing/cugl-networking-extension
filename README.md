# Cornell University Game Library (CUGL) Networking Extension

This repository provides a networking extension to CUGL.

The new classes are all in the `cugl/net` subdirectory; they are `NetworkConnection`,
which provides the actual networking, and the pair `NetworkSerializer` and `NetworkDeserializer`,
which provide a simple way to serialize and deserialize complex data into byte vectors for
the networking class.

This repository acts as a demo app that allows users to click a button and have other
players see how many times they've clicked their button. It has been tested to build on Windows.

Finally, test cases for the serialization classes are in `cugl/lib/test`, under `TCUSerializerTest`.

A NAT Punchthrough server is required to use the networking. See the following repo for setup: 
[https://github.com/mt-xing/nat-punchthrough-server](https://github.com/mt-xing/nat-punchthrough-server)
