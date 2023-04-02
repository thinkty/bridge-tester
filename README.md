
# Bridge Tester

A minimal implementation of the subscriber/publisher client to interact with the [Bridge](https://github.com/thinkty/bridge) server.
Demo can be seen [here](https://youtu.be/4EAAmNV5E1w).

## Build

Clone or download the repository and run `make` to create the executable *publisher* and *subscriber*.

## Usage

### Subscriber

The subscriber needs to handle the *heartbeat* message before receiving the message from the Bridge server to handle dead connections.
Then, the actual message will be posted with 2 bytes of header indicating the size of data to be transmitted.
After all the data has been transmitted from the publisher, to the Bridge server, to the subscriber, the last message will be a *\r\n\r\n* similar to HTTP.

```
./subscriber <ip> <port> <topic> [<file>]
```

The *ip* and *port* are the address of the Bridge server.
The *topic* is the topic to subscribe to.
The subscriber will automatically parse it to a length that is acceptable for the Bridge server.
The optional argument *file* indicate the output file that the contents of the published message will be written to.

### Publisher

Publisher is more simple compared to the subscriber as it only needs to send the message and handle the acknowledgement from the Bridge server just to control the flow between the publisher, Bridge server, and the subscriber.
This step is necessary as the bandwidth may differ (ex. the subscriber may be a low-end device) and we don't want to bombard the network with messages although TCP handles congestion/flow control.

```
./publisher <ip> <port> <topic> <file>
```

Same as the *subscriber*, it takes the *ip* and *port* of the Bridge server and also the topic of interest to publish to.
The last argument *file* is the name of the file that contains the message to publish to the Bridge server.
On a successful operation, the program will output the total number of bytes sent.

## License

MIT
