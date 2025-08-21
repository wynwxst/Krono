#  TCP Protocol documentation

### Introduction
Largely, this TCP implementation is a barebones skeleton held by glue. Amusingly, it has become stable, far more stable than it's occasional quit randomly syndrome. Several causes can be attributed, most of which are documented below.

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