<!--
Copyright (c) 2020-2022 Cogent Embedded, Inc. ALL RIGHTS RESERVED.

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

# Initialization sequence
# Doesn't comply, have to be reworked

```plantuml
@startuml
== Board booted ==
"Application"->"Board Model" ++ :tweak_init_library(endpoint, act_as_server)
"Board Model"->"Board Model" : tweak_wire_init(endpoint)
"Board Model"->"Board Model" : tweak_pickle_init(app_interface, wire_interface)
"Board Model"-->"Application" --

"Application"->"Board Model" ++ :tweak_clear()
note right of "Board Model": initialize data structures
"Board Model"->"GUI Model" ++ : notify_model_altered(revision_id == 0)
note right of "GUI Model": reinitialize data structures
"GUI Model"->"GUI": destroy controls, unbind signals.
"GUI Model" --> "Board Model" --
"Board Model"-->"Application" --
"Application"->"Board Model" ++ :tweak_add_*()
note right of "Board Model": Lock the model, add tweak to list
"Board Model"-->"Application" --
"Board Model"->"GUI Model": notify_model_altered(new revision_id)
"GUI Model"->"GUI Model" ++ : 150ms tick
"GUI Model"->"Board Model" ++ : pull_model(actual revision_id, latest noted revision_id)
"Board Model"->"GUI Model" -- : primary_model_frame
"GUI Model"->"GUI" ++ : Create controls, bind signal
deactivate "GUI"
"GUI Model"->"GUI Model" -- : Unlock new tick

"Application"->"Board Model" ++ :tweak_delete_*()
note right of "Board Model": Lock the model, add deleted tweak placeholder to list
"Board Model"-->"Application" --

== GUI booted ==
"GUI"->"GUI Model" ++ :tweak_init_library(endpoint, act_as_client)
"GUI Model"->"GUI Model" : tweak_wire_init(endpoint)
"GUI Model"->"GUI Model" : tweak_pickle_init(app_interface, wire_interface)
"GUI Model"-->"GUI" -- :
"GUI Model"->"Board Model": pull_model(0, 0)
activate "Board Model"
"Board Model"->"GUI Model": primary_model_frame
deactivate "Board Model"
activate "GUI Model"

note right of "GUI Model" : Lock the model, add tweaks
"GUI Model"->"GUI": Add controls, bind signals
deactivate "GUI Model"
