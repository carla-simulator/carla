import sys
sys.path.insert(0, '.')
from svgkit import Svg, C, render_png
# Server-side sensor pipeline: sensor actor in the Carla plugin -> serializer (LibCarla server) -> stream -> client -> Python.
W, H = 1260, 330
s = Svg(W, H)
s.text(W / 2, 40, "Communication pipeline of a server-side sensor", size=24, weight="bold", anchor="middle")
s.box(30, 110, 200, 90, "Unreal Engine 5.8", ["simulation, rendering"], fill=C["soft_purple"], stroke=C["purple"], tsize=15, lsize=12)
s.box(260, 110, 200, 90, "Carla plugin", ["sensor actor (ASensor)"], fill=C["soft_blue"], stroke=C["blue"], tsize=15, lsize=12)
s.rect(490, 80, 520, 150, fill=C["card"], stroke=C["green"], r=12, sw=2)
s.text(750, 104, "LibCarla", size=16, weight="bold", color=C["green"], anchor="middle")
s.box(510, 120, 150, 90, "Server", ["serializer", "streaming"], fill=C["soft_green"], stroke=C["green"], tsize=15, lsize=12)
s.box(840, 120, 150, 90, "Client", ["deserializer", "SensorData"], fill=C["soft_green"], stroke=C["green"], tsize=15, lsize=12)
s.box(1040, 110, 190, 90, "Python API", ["sensor.listen()"], fill=C["soft_orange"], stroke=C["orange"], tsize=15, lsize=12)
s.line(230, 155, 256, 155, arrow="ink"); s.line(460, 155, 506, 155, arrow="ink")
s.line(660, 165, 836, 165, arrow="ink", dash="6,4"); s.text(748, 155, "TCP stream", size=12, color=C["sub"], anchor="middle")
s.line(990, 155, 1036, 155, arrow="ink")
s.text(W / 2, 290, "The sensor measures in the server, its data is serialized and streamed, and the client turns it back into a SensorData object for the Python callback.",
       size=13, color=C["sub"], anchor="middle", italic=True)
out = sys.argv[1]; s.save(out + '/sensor_pipeline.svg'); render_png(out + '/sensor_pipeline.svg', out + '/sensor_pipeline.png', W, H)
