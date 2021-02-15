classdef Phase
%Phase Construct a Phase object for TraCI traffic lights.
%   myPhase = Phase(DURATION,DURATION1,DURATION2,PHASEDEF) Constructs a 
%   Phase object with the properties DURATION, DURATION1, which is the min 
%   duration in miliseconds; DURATION2, which is the max duration in 
%   miliseconds and PHASEDEF, which is a string of light definitions from 
%   rRgGyYoO for red, green, yellow, off, where lower case letters mean 
%   that the stream has to decelerate.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: Phase.m 49 2018-12-27 14:08:44Z afacostag $

    properties
        duration
        state
        minDur
        maxDur
        next
    end
    methods
        function this = Phase(duration, state, varargin)          
            this.duration = duration;
            this.state = state;
            
            this.minDur = -1;
            this.maxDur = -1;
            this.next = -1;
            
            if nargin > 2
                this.minDur = varargin{1};
                if nargin > 3
                    this.maxDur = varargin{2};
                    if nargin > 4
                        this.next = varargin{3};
                    end
                end
            end
        end
        
        function disp(this)
            disp('Phase:');
            disp(['duration: ' num2str(this.duration)]);
            disp(['state: ' num2str(this.state)]);
            disp(['minDur: ' num2str(this.minDur)]);
            disp(['maxDur: ' num2str(this.maxDur)]);
            disp(['next: ' num2str(this.next)]);
        end
        
    end
end