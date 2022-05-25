<!--
Copyright (c) 2018-2020 Cogent Embedded, Inc. ALL RIGHTS RESERVED.

The source code contained or described herein and all documents related to the
# source code("Software") or their modified versions are owned by
# Cogent Embedded, Inc. or its affiliates.
#
# No part of the Software may be used, copied, reproduced, modified, published,
# uploaded, posted, transmitted, distributed, or disclosed in any way without
# prior express written permission from Cogent Embedded, Inc.
#
# Cogent Embedded, Inc. grants a nonexclusive, non-transferable, royalty-free
# license to use the Software to Licensee without the right to sublicense.
# Licensee agrees not to distribute the Software to any third-party without
# the prior written permission of Cogent Embedded, Inc.
#
# Unless otherwise agreed by Cogent Embedded, Inc. in writing, you may not remove
# or alter this notice or any other notice embedded in Software in any way.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
-->

# Outdated: have to remove 150ms tick

# Software Components

## Terms and Definitions

* Components:
   1. User API is a set of functions from tweak.h accessible for board user
   2. Data model complex data structure consisting of few data arrays and lookup tables. Access is synchronized.
   3. Queue. Synchronized FIFO queue. Additional blocking functions are provided.
   3. Worker thread simple thread that picks tasks from Queue and executex them.
   4. Pickle API presentation layer.
   5. Wire API Asynchronous datagram based IO library.

## Implementation

```plantuml
@startuml
skinparam componentStyle uml2
caption Library components
actor (GUI application)
actor (User application)
package "Wire" {
    [Wire API] --> [Async IO]
    [Async IO] --> [IO completion routine]
}

package "Tweak Application Layer" {
   (150ms Tick) --> [Queue]
   [Model Controller] --> [Queue]
   [Queue] --> (Worker thread)
   [Model Controller] <--> [Data Model]
}

[Qt bindings] <--> [Model Controller]
(GUI application) --> [Qt bindings]

(User application) --> [Tweak user API] : Create model, maintain read only values
[Tweak user API] --> [Model Controller] : Alter model and values
package "Pickle" {
[Pickle API] --> [Wire API]
[Pickle API] --> [Model Controller] : Enqueue inbound requests
(Worker thread) --> [Pickle API] : Enqueue outbound requests
}
[IO completion routine] --> [Pickle API]
@enduml
```

```plantuml
@startuml
skinparam componentStyle uml2
caption Library components
actor (GUI application)
actor (User application)

package "Tweak Application Layer" {
   (150ms Tick) --> [Queue]
   [Model Controller] --> [Queue]
   [Queue] --> (Worker thread)
}

[Qt bindings] <--> [Model Controller]
(GUI application) --> [Qt bindings]

(User application) --> [Tweak user API] : Create model, maintain read only values
[Tweak user API] --> [Model Controller] : Alter model and values
package "Pickle" {

[Pickle API] --> [Model Controller] : Enqueue inbound requests
(Worker thread) --> [Pickle API] : Enqueue outbound requests
}

package "Server Tweak Application Layer" {
   (Server 150ms Tick) --> [Server Queue]
   [Server Model Controller] --> [Server Queue]
   [server Queue] --> (Server Worker thread)
}
@enduml
```

Initialization sequence (Board).

```plantuml
@startuml
participant "User\nApplication"
participant "Tweak\nUser API"
participant "Data\nModel"
participant "150ms Tick"
participant "Queue"
participant "Worker\nThread"
participant "Pickle"
participant "Async\nWire IO"

"User\nApplication" -> "Tweak\nUser API" ++ : tweak_init_library
"Tweak\nUser API" -> "Queue" : Init queue
"Tweak\nUser API" -> "Pickle" : Init pickle
"Pickle" -> "Async\nWire IO" : Init wire
"Tweak\nUser API" -> "Worker\nThread" : Init worker thread
"Worker\nThread"->"Queue"  : Wait job
activate "Queue"
"Tweak\nUser API" -> "Data\nModel" ++ : Init data model
"Tweak\nUser API" --> "User\nApplication" --
"Async\nWire IO" -> "Async\nWire IO": Start receiving
== Incremental update ==
"User\nApplication" -> "Tweak\nUser API" ++ : tweak_add_*
"Tweak\nUser API" -> "Data\nModel" : Data access (synchronized)
"Tweak\nUser API" --> "User\nApplication" -- :
note right of "Data\nModel" : "Updates are collected for 150 ms and send together"
"150ms Tick"--> "Queue" : Post update task
"Queue" -> "Worker\nThread" -- : Fetch send model update task
"Worker\nThread"->"Data\nModel" : Fetch update frame
"Data\nModel" -> "Worker\nThread" -- : Model updates
"Worker\nThread"->"Pickle" ++ : Send model update frame
"Pickle" -> "Async\nWire IO" -- : Enqueue send
"Async\nWire IO" -> "Async\nWire IO" : Send complete
"Async\nWire IO" -> "Async\nWire IO": Start receiving
== GUI just restored connection ==
"Async\nWire IO" -> "Pickle": Pull model datagram
"Pickle" --> "Queue": Post pull model task
"Worker\nThread"->"Queue" ++ : Wait job
"Queue" -> "Worker\nThread" -- : Fetch pull model update task
"Worker\nThread"->"Data\nModel" : Fetch all data
"Data\nModel" -> "Worker\nThread" : Whole model
"Worker\nThread"->"Pickle" ++ : Send model update frame with all data
"Pickle" -> "Async\nWire IO" -- : Enqueue send
"Async\nWire IO" -> "Async\nWire IO" : Send complete
"Async\nWire IO" -> "Async\nWire IO": Start receiving

@enduml
```

Initialization sequence (GUI).

```plantuml
@startuml
participant "GUI\nApplication"
participant "Qt\nbindings"
participant "Data\nModel"
participant "Queue"
participant "Worker\nThread"
participant "Pickle"
participant "Async\nWire IO"

"GUI\nApplication" -> "Qt\nbindings" ++ : tweak_init_library
"Qt\nbindings" -> "Queue" : Init queue
"Qt\nbindings" -> "Pickle" : Init pickle
"Pickle" -> "Async\nWire IO" : Init wire
"Qt\nbindings" -> "Worker\nThread" : Init worker thread
"Qt\nbindings" -> "Data\nModel" : Init data model
"Async\nWire IO" --> "Queue" : Notify online status
"Worker\nThread"->"Queue" ++ : Wait job
"Qt\nbindings" --> "GUI\nApplication" --
"Queue" -> "Worker\nThread" -- : Fetch notify online status task
"Worker\nThread"->"Pickle" ++ : Send pull model
"Pickle" -> "Async\nWire IO" -- : Enqueue send
"Async\nWire IO" -> "Pickle"
"Pickle" -> "Worker\nThread"
"Worker\nThread"->"Queue" : Wait job
"Async\nWire IO" -> "Async\nWire IO" : Send complete
"Async\nWire IO" -> "Async\nWire IO": Start receiving
"Async\nWire IO" -> "Pickle": Model frame datagram
"Pickle" -> "Queue" : Post restore model task
"Worker\nThread"->"Queue" ++ : Wait job
"Queue" -> "Worker\nThread" -- : Fetch restore model task
"Worker\nThread" -> "Qt\nbindings" : Create tweaks
"Qt\nbindings" -> "GUI\nApplication" : Notify model altered
@enduml
```

## Tweak Wire API

### External API
```plantuml
@startuml
actor "User\n(Tweak Pickle)" as user
participant "Tweak Wire" as wire
participant "nng" as net
participant "Remote\nside" as remote

== Init ==
user -> wire ++ : tweak_wire_init(options, callbacks)
wire -> wire : setup structures
wire -> wire : remember callbacks
wire -> net ++ : init()
net -> wire -- : success
wire -> user -- : success

== Connection (Server) ==
user -> wire ++ : tweak_wire_start_server()
wire -> net ++ : nng_pair0_open (create pair endpoint)
wire -> net : nng_aio_alloc() for receive and transmit
wire -> net : nng_listen() create listener object
wire -> user --: success


== Client Connected (Server) ==

net -> wire: on_new_connection()
activate wire
wire -> user : tweak_wire_connection_state_listener()
deactivate wire

== Data Flow (Server) ==

net -> wire ++ : recv_async_callback (receive)
wire -> user -- : tweak_wire_receive_listener
activate user
user -> user : process request
user -> wire ++
deactivate user
wire -> net -- : nng_recv_aio()
...
user -> wire ++ : tweak_wire_transmit()
wire -> net -- : nng_send_aio()
deactivate net

== Connection (Client) ==

user -> wire ++ : tweak_wire_start_client()
wire -> net ++ : nng_pair0_open (create pair endpoint)
wire -> net : nng_aio_alloc() for receive
wire -> net : nng_dial() create connect process
wire -> user --: success

== Connected to Server (Client) ==

net -> wire: on_new_connection()
activate wire
wire -> user : tweak_wire_connection_state_listener()
deactivate wire

== Data Flow (Client) ==

user -> wire ++ : tweak_wire_transmit()
wire -> net -- : nng_send_aio
net -> remote : request

...
remote -> net : response
net -> wire ++ : recv_async_callback (receive)
wire -> user -- : tweak_wire_receive_listener
activate user
user -> user : process response
user -> wire ++
deactivate user
wire -> net -- : nng_recv_aio()
deactivate net

@enduml
```

