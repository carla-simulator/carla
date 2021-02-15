function stage = readStage(result)

result.readInt();
result.read(1);
stageType = result.readInt();
result.read(1);
line = result.readString();
result.read(1);
destStop = result.readString();
result.read(1);
edges = result.readStringList();
result.read(1);
travelTime = result.readDouble();
result.read(1);
cost = result.readDouble();
result.read(1);
intended = result.readString();
result.read(1);
depart = result.readDouble();

stage = struct('stageType', stageType, 'line', line, 'destStop', destStop);
stage.edges = edges;
stage.travelTime = travelTime;
stage.cost = cost;
stage.intended = intended;
stage.depart = depart;
