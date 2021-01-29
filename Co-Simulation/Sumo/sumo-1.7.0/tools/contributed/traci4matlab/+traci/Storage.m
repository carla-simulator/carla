classdef Storage < handle
%Storage A container for responses from SUMO server.

%   Copyright 2019 Universidad Nacional de Colombia,
%   Politecnico Jaime Isaza Cadavid.
%   Authors: Andres Acosta, Jairo Espinosa, Jorge Espinosa.
%   $Id: Storage.m 50 2018-12-28 16:25:47Z afacostag $

	properties
		content
		pos
	end
	methods
		function this = Storage(content)
			this.content = content;
			this.pos = 1;
        end
        
		function value = read(this,numbytes)
			oldpos = this.pos;
			this.pos = this.pos + numbytes;
			value = this.content(oldpos:this.pos-1);
        end
        
        function value = readInt(this)
            value = double(typecast(fliplr(uint8(this.read(4))),'int32'));
        end
        
        function i = readTypedInt(this)
            t = this.read(1);
            assert(t==sscanf(traci.constants.TYPE_INTEGER,'%x'))
            i = double(typecast(fliplr(uint8(this.read(4))),'int32'));
        end
        
        function value = readDouble(this)
            value = typecast(fliplr(this.read(8)),'double');
        end
        
        function i = readTypedDouble(this)
            t = this.read(1);
            assert(t==sscanf(traci.constants.TYPE_DOUBLE,'%x'))
            i = typecast(fliplr(this.read(8)),'double');
        end
        
		function len = readLength(this)
			len = this.read(1);
            if len > 0
                return
            end
            len = this.readInt();
        end
        
        function value = readString(this)
            len = typecast(fliplr(this.read(4)),'int32');
			if len == 0
				value = '';
				return
			end
            value = char(this.read(len));
        end
        
        function value = readTypedString(this)
            t = this.read(1);
            assert(t==sscanf(traci.constants.TYPE_STRING,'%x'))
            value = this.readString();
        end
        
        function stringList = readStringList(this)
            n = this.readInt();
            stringList = cell(1,n);
            for i=1:n
                stringList{i} = this.readString();
            end
        end
        
        function stringList = readTypedStringList(this)
            t = this.read(1);
            assert(t==sscanf(traci.constants.TYPE_STRINGLIST,'%x'))
            stringList = this.readStringList();
        end
        
        function shape = readShape(this)
            len = this.read(1);
            shape = cell(1,len);
            for i=1:len
                shape{i} = typecast([fliplr(this.read(8)) fliplr(this.read(8))],'double');
            end
        end
        
        function s = readCompound(this, varargin)
            if nargin < 2
                expectedSize = [];
            else
                expectedSize = varargin{1};
            end
            t = this.read(1);
            assert(t==sscanf(traci.constants.TYPE_COMPOUND,'%x'))
            s = double(typecast(fliplr(uint8(this.read(4))),'int32'));
            assert(isempty(expectedSize) || s == expectedSize)
        end
        
	end
end
