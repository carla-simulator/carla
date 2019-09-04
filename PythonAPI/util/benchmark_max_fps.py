#!/usr/bin/env python
"""Benchmark Carla."""

import time
import weakref
import carla



class CarlaBenchmarkMaxFPS:
    """Manage and run a Carla world."""

    @classmethod
    def get_arg_parser(cls, parents=None):
        """
        Return a command-line argument parser for this manager class.

        Arguments parsed:
            env_id: environment ID registered with gym of the form [<package>:]<class>-<version>
            agent_id: agent ID of the form <package>[.<module>]:<class>
            render: whether to render (display) environment or not
            print_every: print details every X steps during any episode
            num_timesteps: number of total timesteps to run for
        """
        import argparse

        parser = argparse.ArgumentParser(description=cls.__doc__, parents=(parents or []), add_help=(not parents))
        parser.add_argument(
            '--host', default='localhost',
            help='network host where server is running')
        parser.add_argument(
            '--port', type=int, default=2000,
            help='network port at which server is listening')
        parser.add_argument(
            '--synchronous_mode', dest='synchronous_mode', action='store_true',
            help='enable synchronous_mode on server?')
        parser.add_argument(
            '--asynchronous_mode', dest='synchronous_mode', action='store_false',
            help='disable synchronous_mode on server?')
        parser.add_argument(
            '--rendering_mode', dest='no_rendering_mode', action='store_false',
            help='enable rendering mode on server?')
        parser.add_argument(
            '--no_rendering_mode', dest='no_rendering_mode', action='store_true',
            help='disable rendering mode on server?')
        parser.add_argument(
            '--fps', type=float, default=10.0,
            help='Frames per second (FPS) or Frequency')
        parser.add_argument(
            '--print_every', metavar='P', type=int,
            default=100, help='print details every X steps during any episode')
        parser.add_argument(
            '--num_timesteps', metavar='T', type=int,
            default=1000, help='number of total timesteps to run for')
        parser.set_defaults(synchronous_mode=True, no_rendering_mode=True)
        return parser

    def __init__(self):
        """Initialize based on args"""
        self.args = CarlaBenchmarkMaxFPS.get_arg_parser().parse_args()
        print("carla_benchmark.py: Running with args:", self.args)  # [debug]
        
        # Connect to Carla server, store client 
        self.client = carla.Client(self.args.host, self.args.port)
        print("CarlaBenchmarkMaxFPS(): Connected to server at {}:{}".format(self.args.host, self.args.port))  # [debug]
        self.client.set_timeout(0.1)  
        
        print("Server version : {}".format(self.client.get_server_version()))
        print("Client version : {}".format(self.client.get_client_version()))
            
            
        self.use_old_tick = True if self.client.get_client_version() == "0.9.5" else False
        
        # Initialize clock, timing variables and frame
        self.start_time = time.time()
        self.tick_count = 0
        self.timestamp = None  # received from Carla server through world.wait_for_tick()
        self.sim_fps = 0
        self.server_last_time = time.time()
        self.server_tick_count = 0
        self.simulation_time = 0
        
        
         # Configure world
        self.world = self.client.get_world()
        print("CarlaBenchmarkMaxFPS.load_world(): Configuring world")  # [debug]
        settings = self.world.get_settings()
        settings.synchronous_mode = self.args.synchronous_mode
        settings.no_rendering_mode = self.args.no_rendering_mode
        settings.fixed_delta_seconds = (1.0 / self.args.fps) if  self.args.fps > 0.0 else None
        self.frame = self.world.apply_settings(settings)
        print("CarlaBenchmarkMaxFPS.load_world(): Settings applied; frame: {}".format(self.frame))  # [debug]

        # Fetch map
        self.map = self.world.get_map()
        print("CarlaBenchmarkMaxFPS.load_world(): map: {}".format(self.map))  # [debug]
        
        weak_self = weakref.ref(self)
        self.world.on_tick(lambda timestamp: CarlaBenchmarkMaxFPS.on_world_tick(weak_self, timestamp))
        
    
    @staticmethod
    def on_world_tick(weak_self, timestamp):
        self = weak_self()
        if not self:
            return

        #if timestamp.elapsed_seconds > self.simulation_time:
        self.server_tick_count += 1
        self.simulation_time = timestamp.elapsed_seconds
   
    def run(self):
        """Run Benchmark

        """
        t = None
        try:
           
            # Run each module for num_timesteps
            print("CarlaBenchmarkMaxFPS.run(): Running for {} timesteps...".format(self.args.num_timesteps))  # [debug]
            start_time = time.time()
            last_time = start_time
            for t in range(self.args.num_timesteps):
                
                if self.args.synchronous_mode:
                    self.sync_tick()  
                else:
                    self.world.wait_for_tick(1000.0)

                self.tick_count += 1
                
                # FPS calcualtion and reporting
                if t % self.args.print_every == 0:

                    curr_time = time.time()
                    current_fps = (self.tick_count / (curr_time - last_time))
                    average_fps = (t / (curr_time - start_time))
                    server_fps = (self.server_tick_count / (curr_time - self.server_last_time))

                    report_str = "[t: {:4d}] [CarlaBenchmarkMaxFPS {} mode] [FPS {:.2f} average {:.2f} real/current {:.2f} server {:.2f} sim_step]".format(t, 'sync' if self.args.synchronous_mode else 'async' ,average_fps, current_fps, server_fps, self.sim_fps)

                    print(report_str)  # [debug]

                    last_time = curr_time
                    self.server_last_time  = curr_time
                    self.tick_count = 0
                    self.server_tick_count = 0


        except KeyboardInterrupt:
            print("CarlaBenchmarkMaxFPS.run(): Interrupted; quitting...")  # [debug]
        except Exception as e:
            print("CarlaBenchmarkMaxFPS.run(): Something went wrong; quitting...")  # [debug]
            raise e
        finally:
            # Clean up
            self.close()

            # Calculate elapsed time and steps/sec
            if start_time is not None and t is not None:
                elapsed_time = time.time() - start_time
                print("CarlaBenchmarkMaxFPS.run(): Completed {} timesteps in {:.2f} seconds; avg. steps/sec: {:.2f}".format(
                    t, elapsed_time, t / elapsed_time))  # [debug]
            print("CarlaBenchmarkMaxFPS.run(): Done.")  # [debug]

    def sync_tick(self):
        """Make world tick forward (needed for any actions/changes to be actually executed)."""
        
        if self.use_old_tick:
            self.world.tick()
            self.timestamp = self.world.wait_for_tick(1000.0)
            self.frame += 1
        else:
            # before_tick = time.time()
            self.frame = self.world.tick()
            # after_tick = time.time()
            # elapsed = after_tick-before_tick
            # print("server tick time {}  max possible FPS {:.2f} ".format(elapsed, 1.0/elapsed))
            #time.sleep(0.001)  # sleep to allow Carla server to run for some time
            snapshot = self.world.get_snapshot()
            self.timestamp = snapshot.timestamp
            self.sim_fps = round(1.0 / snapshot.timestamp.delta_seconds)
        
        if self.frame is not None:
            #print('timestamp.frame_count {}'.format(self.timestamp.frame_count))
            if self.timestamp.frame_count != self.frame:
                print('frame skip! timestamp.frame_count {} self.frame {} '.format(self.timestamp.frame_count,self.frame))
            self.frame = self.timestamp.frame_count


    def close(self):
        """Clean up objects and resources."""
        # Clear all actors we created
        if hasattr(self, 'actors'):
            for name in self.actors:
                if not self.actors[name]:
                    continue
                print("CarlaBenchmarkMaxFPS.close(): Clearing out {} actor(s) for '{}'".format(
                    len(self.actors[name]), name))  # [debug]
                for actor in self.actors[name]:
                    actor.destroy()
                self.actors[name].clear()
                # del self.actors[name]  # too harsh? or is it okay to require modules to check on reset?

        # Unset synchronous mode and no_rendering_mode (TODO: only if set?)
        if hasattr(self, 'world'):
            settings = self.world.get_settings()
            settings.synchronous_mode = False
            settings.no_rendering_mode = False
            self.frame = self.world.apply_settings(settings)

        
    def __del__(self):
        self.close()


def main():
    """Run CarlaBenchmarkMaxFPS"""
    
    CarlaBenchmarkMaxFPS().run()


if __name__ == '__main__':
    main()
