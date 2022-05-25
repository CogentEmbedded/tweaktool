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

# Application Protocol
# Doesn't comply, have to be reworked

```plantuml
@startuml
== Connection established ==
"Application"->"Application":Init
"Board Model"->"Board Model":Init
activate "GUI Model"
"GUI Model"-> "Board Model" ++ : request_model
"Board Model"->"GUI Model" -- : model_frame 1..n
deactivate "Board Model"
"GUI Model"->"GUI" ++ : Construct GUI
note right of "GUI": Create widgets
"GUI"-->"GUI Model" -- : bind signals to notify_tweak_altered_interactively
deactivate "GUI Model"
== Change initiated on board side ==
Application -> "Board Model" ++ : tweak_set()
note right of "Board Model": Lock the model, update value, update current frame
"Board Model"-->Application --:
"Board Model"->"Board Model" ++ : 150ms tick
"Board Model"->"GUI Model": periodic_update_frame
activate "GUI Model"
"GUI Model"->"GUI" ++: Lock widgets, emit Qt signals
note right of "GUI": Update widgets
"GUI"-->"GUI Model" --: Unlock widgets
deactivate "GUI Model"
"Board Model"->"Board Model" -- : Unlock next tick
== Change initiated on "GUI" side ==
note right of GUI: Emit Qt signal
"Application"->"Application" ++ : tweak_wait_updates({"Sensor_x", "Sensor_y"}")
note right of "Application": Wait until at least one of tweaks requested by wait call gets updated
GUI->"GUI Model": Qt signal
activate "GUI Model"
"GUI Model"->"Board Model": notify_tweak_altered_interactively
activate "Board Model"
deactivate "GUI Model"
note right of "Board Model": Lock the model, store updated value
"Board Model"->"Application": Send refresh event
note right of "Application": Read updated values
deactivate "Board Model"
deactivate "Application"

@enduml
```

