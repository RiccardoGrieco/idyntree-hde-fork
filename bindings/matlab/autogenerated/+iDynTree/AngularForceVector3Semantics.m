classdef AngularForceVector3Semantics < iDynTree.ForceVector3Semantics__AngularForceVector3Semantics
  methods
    function self = AngularForceVector3Semantics(varargin)
      self@iDynTree.ForceVector3Semantics__AngularForceVector3Semantics(SwigRef.Null);
      if nargin==1 && strcmp(class(varargin{1}),'SwigRef')
        if ~isnull(varargin{1})
          self.swigPtr = varargin{1}.swigPtr;
        end
      else
        tmp = iDynTreeMEX(521, varargin{:});
        self.swigPtr = tmp.swigPtr;
        tmp.SwigClear();
      end
    end
    function varargout = changePoint(self,varargin)
      [varargout{1:nargout}] = iDynTreeMEX(522, self, varargin{:});
    end
    function delete(self)
      if self.swigPtr
        iDynTreeMEX(524, self);
        self.SwigClear();
      end
    end
  end
  methods(Static)
    function varargout = compose(varargin)
     [varargout{1:nargout}] = iDynTreeMEX(523, varargin{:});
    end
  end
end
