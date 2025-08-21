#  TCP Protocol documentation

### Introduction
Largely, this TCP implementation is a barebones skeleton held by glue. Amusingly, it has become stable, far more stable than it's initial quit randomly syndrome. Several causes for this stability can be attributed, most of which are documented below.

### Handshake
Effectively the handshake between server and client is as follows.

**1) Len packet**

A message of `len:<8 digits>` is made, with padding of `0`s if the length is less than 8, to signify the length of the on-coming message.

If the length is passed correctly, the receiver will send `ok` back as a sanity procotol.

**2) Receiving**
Now that the length of the incoming message is confirmed, the receiver will simply receive up to that length.

**3) Full Process**

The full process, put together is (example values are used):

`sender -> len:00000011 -> receiver`

`receiver -> 'ok' -> sender`

`sender -> 'Hello World' -> receiver`

This handshake is used to send any message whatsoever.

### Commands
Basic command protocol is as follows:

`client -> command_name eg. 'ping' -> server`

**1) No Arguments Required**

`server processes command, if the command exists; -> 'return result' -> client`

**2) Arguments Required**

`server processes command, if the command exists; -> 'ok' -> client`

`client -> arguments -> server`

`server -> 'return result' -> client`

### Panels
Basic panel protocol:

**1) No Updates Required - Static at launch time**

`panel client -> <panel specific command request> -> server`

`server -> <panel specific command response> -> panel`

`end of transmission`

**2) Updates required**

At the start:

`panel client -> <panel specific command request> -> server`

`server -> <panel specific command response> -> panel`

As a regular task:

`panel client -> 'pendingupdate' -> server`

`server -> 'no' -> panel client (return to pending update request)`

or (Update sequence)

`server -> 'yes' -> panel client`

`panel client -> <panel specific command request> -> server`

`server -> <panel specific command response> -> panel`

`panel -> 'updated' -> server`

or

`server -> 'specific' -> panel client`

`panel client -> <panel name> -> server`

`server -> yes/no -> panel client (if yes, begin update sequence, if no loop back)`

### Console
The console has two main variants of communication and one special form.

Regular:

The console displays any message sent to it by the server (perceived as STDOUT) 

This is used to display results of commands from the console, mainly.

LPAD (Loop and display):

This is largely considered as the process' running space

`server -> 'LPAD' -> console client` (Instead of 'ok')

`server -> 'startloop' -> console client`

`server -> <STDOUT from regular running> -> console client`

`server -> 'endloop' -> console client`

Input (Special protocol during LPAD):

`server -> 'input_required' -> console client`

`server -> 'begin_prompt' -> console client`

`server -> <Feeds flushed STDOUT> -> console client`

`server -> 'end_prompt' -> console client`

During this time, the process thread spins, waiting for the client to send the input

`console client -> <set_input command with console stdin> -> server`

`server -> 'received' -> client`

`console client -> <free_input command> -> server`

This stops the process from spinning.



