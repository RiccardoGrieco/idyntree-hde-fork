classdef AngularForceVector3 < iDynTree.ForceVector3__AngularForceVector3
  methods
    function self = AngularForceVector3(varargin)
      self@iDynTree.ForceVector3__AngularForceVector3(SwigRef.Null);
      if nargin==1 && strcmp(class(varargin{1}),'SwigRef')
        if varargin{1}~=SwigRef.Null
          self.swigPtr = varargin{1}.swigPtr;
        end
      else
        tmp = iDynTreeMEX(317, varargin{:});
        self.swigPtr = tmp.swigPtr;
        tmp.swigPtr = [];
      end
    end
    function delete(self)
      if self.swigPtr
        iDynTreeMEX(318, self);
        self.swigPtr=[];
      end
    end
    function varargout = changePoint(self,varargin)
      [varargout{1:nargout}] = iDynTreeMEX(319, self, varargin{:});
    end
  end
  methods(Static)
  end
end
