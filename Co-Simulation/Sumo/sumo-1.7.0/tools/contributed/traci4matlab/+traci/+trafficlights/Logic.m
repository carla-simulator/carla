classdef Logic
%Logic constructs a Logic object.
%   myLogic = Logic(SUBID,TYPE,SUBPARAMETER,CURRENTPHASEINDEX,PHASES)
%   constructs a Logic object myLogic containing an ID SUBID, astarting 
%   phase index CURRENTPHASEINDEX, and a cell array of traci.Phase objects,
%   PHASES, which represent the phase definitions of trafic lights' program
%   myLogic. TYPE and SUBPARAMETER are not currently implemented in SUMO 
%   server, therefore, the must be set to zero.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: Logic.m 49 2018-12-27 14:08:44Z afacostag $

    properties
        programID
        type
        currentPhaseIndex
        phases
        subParameter
    end
    methods
        function this = Logic(programID, type, currentPhaseIndex, varargin)            
            this.programID = programID;
            this.type = type;
            this.currentPhaseIndex = currentPhaseIndex;
            
            this.phases = {};
            this.subParameter = containers.Map();
            
            if nargin > 3
                this.phases = varargin{1};
                if nargin > 4
                    this.subParameter = varargin{2};
                end
            end
                
        end
        
        function disp(this)
            disp('Logic:');
            disp(['programID: ' num2str(this.programID)]);
            disp(['type: ' num2str(this.type)]);
            disp(['currentPhaseIndex: ' num2str(this.currentPhaseIndex)]);
            for i=1:length(this.phases)
                disp(this.phases{i});
            end
            disp(['subParameter: ' num2str(this.subParameter)]);
        end
        
    end
end