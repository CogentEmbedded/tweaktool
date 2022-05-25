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

## Server connection state tracker

```plantuml
@startuml
participant "API"
participant "Model-Controller"
participant "Item indices"
participant "Item storage"
participant "Queue"
participant "Worker"
participant "Pickle"

"Worker"->"Queue"++: Wait job
note right of "Model-Controller": connected == false
"API" -> "Model-Controller": add_item
"Model-Controller"->"Item indices": Acquire write lock
"Model-Controller"->"Model-Controller": add item and update indices
note right of "Model-Controller": add requests aren't\npropagated to Queue unless\nconnected == true
"Model-Controller"->"Item indices": Release write lock
"Model-Controller"->"API"

"API" -> "Model-Controller": copy_item
"Model-Controller"->"Item indices": Acquire read lock
"Model-Controller"->"Model-Controller": Lookup index and get item reference
"Model-Controller"->"Item storage": Acquire lock
"Model-Controller"->"Item storage": Copy item value
"Model-Controller"->"Item storage": Release lock
"Model-Controller"->"Item indices": Release read lock
"API" <- "Model-Controller": Pass item ownership

"API" -> "Model-Controller": replace_item
"Model-Controller"->"Item indices": Acquire read lock
"Model-Controller"->"Model-Controller": Lookup index, find item
"Model-Controller"->"Item storage": Acquire lock
"Model-Controller"->"Item storage": Replace item value
"Model-Controller"->"Item storage": Release lock
"Model-Controller"->"Model-Controller": Check connected status
note right of "Model-Controller": Change requests aren't\npropagated to Queue unless connected == true

"Model-Controller"->"Item indices": Release read lock

"Model-Controller"->"Model-Controller": Release previous item value
"API" <- "Model-Controller"

"API" -> "Model-Controller": remove_item
"Model-Controller"->"Item indices": Acquire write lock
"Model-Controller"->"Model-Controller": remove item, free memory and update indices
note right of "Model-Controller": remove requests aren't\npropagated to Queue unless\nconnected == true
"Model-Controller"->"Item indices": Release write lock
"Model-Controller" -> "API"

== Failed connect attempt ==

"Pickle"->"Model-Controller" : Subscribe request
note right of "Model-Controller": connected <- true
"Model-Controller"->"Queue": Post subscribe job
"Queue"->"Worker"--: Fetch job
activate "Worker"
"Worker"->"Item indices": Acquire read lock
"Worker"->"Item storage": Acquire lock
"Worker"->"Model-Controller": Fetch item
"Worker"->"Worker": Prepare request
"Worker"->"Item storage": Release lock
"Worker"->"Pickle": Send request
"Worker"->"Worker": Fetch next job
"Worker"->"Item storage": Acquire lock
"Worker"->"Model-Controller": Fetch item
"Worker"->"Worker": Prepare request
"Worker"->"Item storage": Release lock
"Worker"->"Pickle": Send request
"Worker"->"Worker": Fetch next job
...
"API" -> "Model-Controller": replace_item
"Model-Controller"->"Item indices": Acquire read lock
"Model-Controller"->"Model-Controller": Lookup index, find item
"Model-Controller"->"Item storage": Acquire lock
"Model-Controller"->"Item storage": Replace item value
"Model-Controller"->"Item storage": Release lock
"Model-Controller"->"Model-Controller": Check connected status
note right of "Model-Controller": Assuming that Queue is empty\nupon subscription and have enough slots to\npostpone change requests until subscription is complete
"Model-Controller"->"Queue": Post change job
"Model-Controller"->"Item indices": Release read lock
"Model-Controller"->"Model-Controller": Release previous item value
"API" <- "Model-Controller"
"API" -> "Model-Controller": add or remove item
"Model-Controller"-> "Item indices": Acquire write lock
activate "Item indices"
"Pickle"->"Model-Controller" : Disconnected
note right of "Model-Controller": connected <- false
"Model-Controller"->"Queue": Empty queue
"Worker"->"Model-Controller": Fetch item
"Worker"->"Worker": Prepare request
"Worker"->"Item storage": Release lock
"Worker"->"Pickle": Send request
"Pickle"->"Worker": RPC operation failed
"Worker"->"Worker": Skipping remaining add_item requests
"Worker"->"Item indices": Release read lock
deactivate "Worker"
"Worker"->"Queue"++: Wait job
"Item indices"->"Model-Controller": Got write access to model data
deactivate "Item indices"
"Model-Controller"->"Model-Controller": Create new item, update indices
"Model-Controller"->"Model-Controller": Check connected status
note right of "Model-Controller": Add/Remove requests aren't\npropagated to Queue unless connected == true
"Model-Controller"->"Item indices": Release write lock
"Model-Controller"->"API"

== Client connected==

"Pickle"->"Model-Controller" : Subscribe request
note right of "Model-Controller": connected <- true
"Model-Controller"->"Queue": Post subscribe job
"Queue"->"Worker"--: Fetch job
activate "Worker"
"Worker"->"Item indices": Acquire read lock
"Worker"->"Item storage": Acquire lock
"Worker"->"Model-Controller": Fetch item
"Worker"->"Worker": Prepare request
"Worker"->"Item storage": Release lock
"Worker"->"Pickle": Send request
"Worker"->"Worker": Fetch next job
"Worker"->"Item storage": Acquire lock
"Worker"->"Model-Controller": Fetch item
"Worker"->"Worker": Prepare request
"Worker"->"Item storage": Release lock
"Worker"->"Pickle": Send request
"Worker"->"Worker": Fetch next job
...
"API" -> "Model-Controller": replace_item
"Model-Controller"->"Item indices": Acquire read lock
"Model-Controller"->"Model-Controller": Lookup index, find item
"Model-Controller"->"Item storage": Acquire lock
"Model-Controller"->"Item storage": Replace item value
"Model-Controller"->"Item storage": Release lock
"Model-Controller"->"Model-Controller": Check connected status
note right of "Model-Controller": Assuming that Queue is empty\nupon subscription and have enough slots to\npostpone change requests until subscription is complete
"Model-Controller"->"Queue": Post change job
"Model-Controller"->"Item indices": Release read lock
"Model-Controller"->"Model-Controller": Release previous item value
"API" <- "Model-Controller"
"API" -> "Model-Controller": add or remove item
"Model-Controller"-> "Item indices": Acquire write lock
activate "Item indices"
"Worker"->"Worker": Last job
deactivate "Worker"
"Worker"->"Item indices": Release read lock
"Item indices" -> "Model-Controller"
deactivate "Item indices"
"Model-Controller"->"Model-Controller": Create new item, update indices
"Model-Controller"->"Model-Controller": Check connected status
"Model-Controller"->"Queue": Post add_item job
"Model-Controller"->"Item indices": Release write lock
"Model-Controller"->"API"
== Client subscribed ==
"Worker"->"Queue" ++: Fetch jobs
"API" -> "Model-Controller": add_item
"Model-Controller"->"Item indices": Acquire write lock
"Model-Controller"->"Queue": Post add item job
"Model-Controller"->"Item indices": Release write lock
"Model-Controller"->"API"

"API" -> "Model-Controller": replace_item
"Model-Controller"->"Item indices": Acquire read lock
"Model-Controller"->"Model-Controller": Lookup index, find item
"Model-Controller"->"Item storage": Acquire lock
"Model-Controller"->"Item storage": Replace item value
"Model-Controller"->"Item storage": Release lock
"Model-Controller"->"Queue": Post change item job
"Model-Controller"->"Item indices": Release read lock
"Model-Controller"->"Model-Controller": Release previous item value
"API" <- "Model-Controller"

"Queue"->"Worker"--: Job batch
activate "Worker"
"Worker"->"Worker": Prepare pickle requests
"Worker"->"Pickle": Perform requests
deactivate "Worker"

"Worker"->"Queue" ++: Fetch jobs
"API" -> "Model-Controller": remove_item
"Model-Controller"->"Item indices": Acquire write lock
"Model-Controller"->"Model-Controller": remove item, free memory and update indices
"Model-Controller"->"Queue": Post remove item job
"Queue"->"Worker"--: Job batch
activate "Worker"
"Worker"->"Worker": Prepare pickle requests
"Worker"->"Pickle": Perform requests
deactivate "Worker"
"Model-Controller"->"Item indices": Release write lock
"Model-Controller"->"API"

== Server return to standalone mode ==

note right of "Model-Controller": connected <- false

@enduml
```

## Client connection state tracker

# Doesn't comply, have to be reworked

```plantuml
@startuml
participant "Model-Controller"
participant "Queue"
participant "Worker"
participant "Pickle"

"Worker"->"Queue"++: Wait job
note right of "Model-Controller": connected == false
"Pickle"->"Model-Controller" : state <- connected
"Model-Controller"->"Queue": Post subscribe job
"Queue"->"Worker"--: Fetch job
"Worker"->"Pickle": Subscribe
"Pickle"->"Model-Controller": add_item
"Model-Controller"->"Model-Controller": state <- subscribed
"Pickle"->"Model-Controller" : state <- disconnected
"Model-Controller"->"Queue": Purge jobs
@enduml
```



```plantuml
@startuml

() api2
() wire
() pickle
() model

set namespaceSeparator ::

class Tweak::server::controller {
  add_item(meta, initial_value): id
  find_id_by_uri(uri): id
  get_item_float(id): float
  get_item_meta(id): meta
  set_item_float(id, value): void
  remave_item(id)
}

class Tweak::model::model {
  append_item(item)
  find_item_by_id(id): item
  find_item_id_by_uri(uri): id
  set_current_value(id, value)
  remove_item(id)
}

class Tweak::model::uri2id_index {
  add_uri_id_pair(uri, id)
  find_item_id_by_uri(uri): id
  remove_uri(uri): id
}

class Tweak::server::worker {
  submit_item_appended_task(item)
  submit_item_changed_task(id, value)
  submit_item_removed_task(id)
}

class Tweak::pickle::server_endpoint {
  on_subscribe: event
  on_item_changed: event
  add_item(item)
  change_item(id,value)
  remove_item(id)
}

api2 - Tweak::server::controller
Tweak::model::model - model
Tweak::model::model - Tweak::model::uri2id_index
Tweak::server::controller - Tweak::server::worker
Tweak::server::controller -> model
Tweak::pickle::server_endpoint - pickle
Tweak::server::controller -> pickle
Tweak::server::worker -> pickle
Tweak::pickle::server_endpoint -> wire

@enduml
```

## Initialization sequence (Board, Tweak-api2 user).

```plantuml
@startuml
participant "Tweak-api2"
participant "Model-Controller"
participant "Model-Data"
participant "Queue"
participant "Worker\nThread"
participant "Pickle"

"Tweak-api2"->"Model-Controller" ++ : tweak_init_library()
"Model-Controller"->"Model-Data": tweak_model_create()
"Model-Controller"->"Queue" : tweak_queue_create()
"Model-Controller"->"Worker\nThread" : tweak_worker_create()
"Worker\nThread"->"Queue" ++ : wait_task()
"Model-Controller"->"Pickle" : tweak_pickle_create()
"Model-Controller"->"Tweak-api2" -- :
"Tweak-api2"->"Model-Controller" ++ : tweak_add_...()
"Model-Controller"->"Model-Data" : tweak_model_add()
"Model-Controller"->"Model-Controller": has_subscriptions == False
"Model-Controller"->"Tweak-api2" --

"Tweak-api2"->"Model-Controller" ++ : tweak_add_...()
"Model-Controller"->"Model-Data" : tweak_model_add()
"Model-Controller"->"Model-Controller": has_subscriptions == False
"Model-Controller"->"Tweak-api2" --

"Pickle"->"Model-Controller": subscribe
"Model-Controller"->"Model-Data" ++ : lock model
note right of "Model-Data": All tweak_model_*\nrequests will block\nuntil client\nis synchronized
"Model-Controller"->"Model-Data": walk()
"Model-Controller"->"Queue": submit(add_item_task) 1..n
"Model-Data"->"Model-Controller" -- : unlock model
"Model-Controller"->"Model-Controller": has_subscriptions <- True
"Queue" -> "Worker\nThread" -- : fetch_task()
"Worker\nThread"->"Model-Data"  : Access items 1..n
activate "Worker\nThread"
note left of "Worker\nThread": Model is locked when worker\nconverts items to pickle format
"Worker\nThread"->"Worker\nThread": Convert items to pickle format
"Model-Data"->"Worker\nThread"  :
"Worker\nThread"->"Pickle" : Propagate requests to pickle
deactivate "Worker\nThread"
"Worker\nThread"->"Queue" ++ : wait_task()
"Tweak-api2"->"Model-Controller" ++ : tweak_add_...()
"Model-Controller"->"Model-Data" : tweak_model_add()
"Model-Controller"->"Queue": if (has_subscriptions) submit(add_item_task)
"Model-Controller"->"Tweak-api2" --
"Queue" -> "Worker\nThread" -- : fetch_task()
"Worker\nThread"->"Model-Data" : Access item
activate "Worker\nThread"
"Worker\nThread"->"Worker\nThread": Convert items to pickle format
"Model-Data"->"Worker\nThread"  :
"Worker\nThread"->"Pickle" : Propagate requests to pickle
deactivate "Worker\nThread"

"Worker\nThread"->"Queue" ++ : wait_task()

"Tweak-api2"->"Model-Controller" ++ : tweak_set_...()
"Model-Controller"->"Model-Data" : tweak_model_set()
"Model-Controller"->"Queue": if (has_subscriptions) submit(change_item_task)
"Model-Controller"->"Tweak-api2" --

"Queue" -> "Worker\nThread" -- : fetch_task()
"Worker\nThread"->"Model-Data"  : Access item
activate "Worker\nThread"
"Worker\nThread"->"Worker\nThread": Convert items to pickle format
"Model-Data"->"Worker\nThread" :
"Worker\nThread"->"Pickle" : Propagate requests to pickle
deactivate "Worker\nThread"

"Worker\nThread"->"Queue" ++ : wait_task()

"Tweak-api2"->"Model-Controller" ++ : tweak_remove()
"Model-Controller"->"Model-Data" : tweak_model_remove()
"Model-Controller"->"Queue": if (has_subscriptions) submit(remove_task)
"Model-Controller"->"Tweak-api2" --
"Queue" -> "Worker\nThread" -- : fetch_task()
"Worker\nThread"->"Pickle" : Propagate requests to pickle

"Worker\nThread"->"Queue" ++ : wait_task()

"Pickle"->"Model-Controller": item_change
activate "Model-Controller"
"Model-Controller"->"Model-Data" : tweak_model_set()
"Model-Controller"->"Tweak-api2": on_item_changed
deactivate "Model-Controller"

"Tweak-api2"->"Model-Controller" ++ : tweak_get_...()
"Model-Controller"->"Model-Data" : tweak_model_get()
"Model-Controller"->"Tweak-api2" --

@enduml
```

## Client components.

```plantuml
@startuml

() client_api2
() wire

set namespaceSeparator ::

class Tweak::client::controller {
  on_item_added: event
  on_item_changed: event
  on_item_removed: event
  find_id_by_uri(uri): id
  get_item_float(id): float
  get_item_meta(id): meta
  set_item_float(id, value): void
}

client_api2 -> Tweak::client::controller : Expose

class Tweak::model::model {
  add_item(item)
  find_item_by_id(id): item
  find_item_id_by_uri(uri): id
  set_current_value(id, value)
  remove_item(id)
}

class Tweak::model::uri2id_index {
  add_uri_id_pair(uri, id)
  find_item_id_by_uri(uri): id
  remove_uri(uri): id
}

Tweak::model::model -> Tweak::model::uri2id_index : Use

class Tweak::client::worker {
  submit_subscribe_task()
  submit_item_changed_task(id, value)
}

Tweak::client::controller -> Tweak::client::worker : Submit tasks
Tweak::client::controller -> Tweak::model::model : Keep in sync

class Tweak::pickle::client_endpoint {
  on_client_connected: event
  on_item_added: event
  on_item_changed: event
  on_item_removed: event
  subscribe(uri_pattern)
  change_item(id, value)
}

Tweak::pickle::client_endpoint -> Tweak::client::controller : Trigger inbound events
Tweak::client::worker -> Tweak::pickle::client_endpoint : Submit tasks

Tweak::pickle::client_endpoint->wire : Use

@enduml
```
## Initialization sequence (Client).

```plantuml
@startuml
participant "Qt user"
participant "Model-Controller"
participant "Model-Data"
participant "Queue"
participant "Worker\nThread"
participant "Pickle"

"Qt user"->"Model-Controller" ++ : tweak_qt_bridge_create_replica_model_instance(backend, uri, params)
"Model-Controller"->"Model-Data" : create
"Model-Controller"->"Queue" : create
"Model-Controller"->"Worker\nThread" : create
"Worker\nThread"->"Queue" ++ : wait_task()
"Model-Controller"->"Worker\nThread" : create
"Model-Controller"->"Qt user" -- : model_instance
"Pickle"->"Model-Controller" : notify connected
"Model-Controller"->"Queue": submit(subscribe_task)
"Queue"->"Worker\nThread" --: fetch_task()
"Worker\nThread"->"Pickle": send submit()
"Worker\nThread"->"Queue" ++ : wait_task()
"Pickle"->"Model-Controller": add_item
"Model-Controller"->"Model-Data": create replica items
"Model-Controller"->"Qt user": emit add_item(model_instance, item)
"Qt user"->"Model-Controller": bind or unbind
"Qt user"->"Model-Controller" ++: emit item updated
"Model-Controller"->"Model-Data": update
"Model-Controller"->"Queue" -- : enqueue(change_item_task)
"Queue"->"Worker\nThread" --: fetch_task()
"Worker\nThread"->"Model-Data" : Access item
activate "Worker\nThread"
"Worker\nThread"->"Worker\nThread": Convert items to pickle format
"Model-Data"->"Worker\nThread" :
"Worker\nThread"->"Pickle" : Propagate requests to pickle
deactivate "Worker\nThread"
"Worker\nThread"->"Queue" ++ : wait_task()
"Pickle"->"Model-Controller": change_item
activate "Model-Controller"
"Model-Controller"->"Model-Data": update
"Model-Controller"->"Qt user": emit item updated(model_instance, item)
"Qt user"->"Model-Controller": emit item updated
"Model-Controller"->"Model-Controller": suppress echo update
deactivate "Model-Controller"
deactivate "Queue"

@enduml
```

