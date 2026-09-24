import sys,unittest,xml.etree.ElementTree as ET
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'scenic'))
from scenic_xodr import repair_width_undershoot, sanitize

def fixture(a,b,c,d):
 return ET.fromstring(f'<OpenDRIVE><road id="1" length="1"><lanes><laneSection s="0"><right><lane id="-1"><width sOffset="0" a="{a}" b="{b}" c="{c}" d="{d}" /></lane></right></laneSection></lanes></road></OpenDRIVE>')

class WidthRepairTest(unittest.TestCase):
 def test_interior_negative_with_positive_endpoints(self):
  root=fixture(.0005,-.01,.01,0);r=repair_width_undershoot(root)
  self.assertAlmostEqual(r[0]['correction_m'],.002000001)
  a=float(root.find('.//width').get('a'))
  self.assertGreater(a-.01*.5+.01*.5**2,0)
  self.assertEqual(repair_width_undershoot(root),[])
 def test_refuse_large_layout_change(self):
  with self.assertRaisesRegex(ValueError,'exceeds repair tolerance'):repair_width_undershoot(fixture(-.021,0,0,0))
 def test_valid_width_unchanged(self):
  root=fixture(3.5,.1,0,0);before=ET.tostring(root)
  self.assertEqual(repair_width_undershoot(root),[]);self.assertEqual(ET.tostring(root),before)
 def test_zero_width_lane_is_not_a_driving_polygon(self):
  root=fixture(0,0,0,0);root.find('.//lane').set('type','driving')
  report=repair_width_undershoot(root)
  self.assertTrue(report[0]['excluded_zero_width'])
  self.assertEqual(root.find('.//lane').get('type'),'none')

class JunctionRepairTest(unittest.TestCase):
 def network(self, links):
  def road(rid,junction,roadlinks,lanes):
   return f'<road id="{rid}" junction="{junction}" length="10"><link>{roadlinks}</link><lanes><laneSection s="0"><right>{lanes}</right></laneSection></lanes></road>'
  def lane(lid, successor=''):
   return f'<lane id="{lid}" type="driving"><link>{successor}</link><width sOffset="0" a="3.5" b="0" c="0" d="0"/></lane>'
  return '<OpenDRIVE>'+road('1','-1','<successor elementType="junction" elementId="4"/>',lane('-1')+lane('-3'))+road('2','4','<predecessor elementType="road" elementId="1" contactPoint="end"/><successor elementType="road" elementId="3" contactPoint="start"/>',lane('-1','<successor id="-1"/>'))+road('3','-1','',lane('-1'))+f'<junction id="4"><connection id="0" incomingRoad="1" connectingRoad="2" contactPoint="start">{links}</connection></junction></OpenDRIVE>'
 def test_drop_missing_explicit_target(self):
  clean,report=sanitize(self.network('<laneLink from="-1" to="-1"/><laneLink from="-3" to="-3"/>'))
  self.assertEqual(len(ET.fromstring(clean).findall('.//laneLink')),1)
  self.assertEqual(report['dropped_junction_lane_links'][0]['to_lane'],'-3')
 def test_empty_links_only_materialize_valid_identity(self):
  clean,report=sanitize(self.network(''))
  links=ET.fromstring(clean).findall('.//laneLink')
  self.assertEqual([(l.get('from'),l.get('to')) for l in links],[('-1','-1')])
  self.assertTrue(report['dropped_junction_lane_links'][0]['implicit_identity'])
 def test_all_invalid_dissolves_instead_of_implicit_fallback(self):
  clean,report=sanitize(self.network('<laneLink from="-3" to="-3"/>'))
  self.assertFalse(ET.fromstring(clean).findall('junction'))
  self.assertEqual(report['dissolved_junctions'][0]['id'],'4')

if __name__=='__main__':unittest.main()
