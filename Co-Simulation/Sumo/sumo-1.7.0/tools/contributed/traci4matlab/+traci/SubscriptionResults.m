classdef SubscriptionResults < handle
%SubscriptionResults A container for the subscription results.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: SubscriptionResults.m 48 2018-12-26 15:35:20Z afacostag $

    properties
        results
        contextResults
        valueFunc
    end
    
    methods
        function this = SubscriptionResults(valueFunc)
            this.results = containers.Map;
            this.contextResults = containers.Map;
            this.valueFunc = valueFunc;
        end

        function result = parse(this, varID, data)
            if ~isKey(this.valueFunc, varID)
                throw(MException('traci:FatalTraCIError', 'Unknown variable %.2x',varID))
            end
            resultHandle = str2func(this.valueFunc(varID));
            result = resultHandle(data);
        end

        function reset(this)
            this.results = containers.Map;
            this.contextResults = containers.Map;
        end

        function add(this, refID, varID, data)
            if ~isKey(this.results, refID)
                this.results(refID) = containers.Map;
            end
            handle1results = this.results(refID);
            handle1results(varID) = this.parse(varID, data);
        end
    
        function subscriptionResults = get(this, refID)
            refID = num2str(refID);
            if nargin < 2
                refID = 'None';
            end
            if strcmp(refID,'None') == 1
                subscriptionResults = this.results;
                return
            end
            if isKey(this.results, refID)
                subscriptionResults = this.results(refID);
            else
                subscriptionResults = 'None';
            end
        end

        function addContext(this, refID, domain, objID, varID, data)
            if ~isKey(this.contextResults, refID)
                this.contextResults(refID) = containers.Map;
            end
            if ~isKey(this.contextResults(refID), objID)
                this.contextResults(refID) = subsasgn(this.contextResults(refID),...
                    struct('type','()','subs',{objID}),containers.Map);
            end
            handle1ContextResults = this.contextResults(refID);
            handle2ContextResults = handle1ContextResults(objID);
            handle2ContextResults(varID) = domain.parse(varID, data);
        end
        
        function context = getContext(this, refID)
            if nargin < 2
                refID = 'None';
            end
            if strcmp(refID, 'None') == 1
                context = this.contextResults;
                return
            end
            if isKey(this.contextResults, refID)
                context = this.contextResults(refID);
            else
                context = 'None';
            end
        end
    end
end