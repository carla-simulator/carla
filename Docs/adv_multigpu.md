# Carla Multi-GPU feature

Multi-GPU in carla means that the user can  start several servers (called secondary servers) that will do render work for the main server (called primary server) for different sensors.

## Primary server

The steps are: first, start the primary server without any render capability. The parameters we can use are:

* `-nullrhi`
* `-carla-primary-port`

Run the following from the command line:

```sh
./CarlaUE4.sh -nullrhi
```

The primary server will use, by default, the port 2002 to listen for secondary servers. If you need to listen on another port, then you can change it with the port flag:

```sh
./CarlaUE4.sh -nullrhi -carla-primary-port=3002
```

## Secondary servers

We may then start as many servers as we need, but the ideal is to have as many secondary servers as the number of GPUs installed in the computer. Through parameters we need to specify the GPU we want the server use and also the host/port where the primary server is listening, with the flags:

  * `-carla-rpc-port`
  * `-carla-primary-host`
  * `-carla-primary-port`
  * `-ini:[/Script/Engine.RendererSettings]:r.GraphicsAdapter`

For example, if the primary server is executing in the same computer than the secondary servers and with the default port, we can use this command:

```sh
./CarlaUE4.sh -carla-rpc-port=3000 -carla-primary-host=127.0.0.1 -ini:[/Script/Engine.RendererSettings]:r.GraphicsAdapter=0
```

Here, the secondary server will use port 3000 as the RPC server to avoid conflicts with other ports (but it will never be used), and will connect to the primary server located at IP 127.0.0.1 (localhost) in the default port (2002), and also this server will use the GPU device 0.

If we want to start another secondary server using another GPU, we could use the following command:

```sh
./CarlaUE4.sh -carla-rpc-port=4000 -carla-primary-host=127.0.0.1 -carla-primary-port=2002 -ini:[/Script/Engine.RendererSettings]:r.GraphicsAdapter=1
```

This secondary server will use port 4000 as the RPC server to avoid conflicts with other ports and will connect to the primary server located at IP 127.0.0.1 in port 2002, and also this server will use the GPU device 1.

## Pipeline

After the first secondary server connects to the primary server, it will set up the synchronous mode automatically, with the default values of 1/20 delta seconds.

