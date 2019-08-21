classdef BerdySparseMAPSolver < SwigRef
  methods
    function this = swig_this(self)
      this = iDynTreeMEX(3, self);
    end
    function self = BerdySparseMAPSolver(varargin)
      if nargin==1 && strcmp(class(varargin{1}),'SwigRef')
        if ~isnull(varargin{1})
          self.swigPtr = varargin{1}.swigPtr;
        end
      else
        tmp = iDynTreeMEX(1612, varargin{:});
        self.swigPtr = tmp.swigPtr;
        tmp.SwigClear();
      end
    end
    function delete(self)
      if self.swigPtr
        iDynTreeMEX(1613, self);
        self.SwigClear();
      end
    end
    function varargout = setDynamicsConstraintsPriorCovariance(self,varargin)
      [varargout{1:nargout}] = iDynTreeMEX(1614, self, varargin{:});
    end
    function varargout = setDynamicsRegularizationPriorCovariance(self,varargin)
      [varargout{1:nargout}] = iDynTreeMEX(1615, self, varargin{:});
    end
    function varargout = setDynamicsRegularizationPriorExpectedValue(self,varargin)
      [varargout{1:nargout}] = iDynTreeMEX(1616, self, varargin{:});
    end
    function varargout = setMeasurementsPriorCovariance(self,varargin)
      [varargout{1:nargout}] = iDynTreeMEX(1617, self, varargin{:});
    end
    function varargout = dynamicsConstraintsPriorCovarianceInverse(self,varargin)
      [varargout{1:nargout}] = iDynTreeMEX(1618, self, varargin{:});
    end
    function varargout = dynamicsRegularizationPriorCovarianceInverse(self,varargin)
      [varargout{1:nargout}] = iDynTreeMEX(1619, self, varargin{:});
    end
    function varargout = dynamicsRegularizationPriorExpectedValue(self,varargin)
      [varargout{1:nargout}] = iDynTreeMEX(1620, self, varargin{:});
    end
    function varargout = measurementsPriorCovarianceInverse(self,varargin)
      [varargout{1:nargout}] = iDynTreeMEX(1621, self, varargin{:});
    end
    function varargout = isValid(self,varargin)
      [varargout{1:nargout}] = iDynTreeMEX(1622, self, varargin{:});
    end
    function varargout = initialize(self,varargin)
      [varargout{1:nargout}] = iDynTreeMEX(1623, self, varargin{:});
    end
    function varargout = updateEstimateInformationFixedBase(self,varargin)
      [varargout{1:nargout}] = iDynTreeMEX(1624, self, varargin{:});
    end
    function varargout = updateEstimateInformationFloatingBase(self,varargin)
      [varargout{1:nargout}] = iDynTreeMEX(1625, self, varargin{:});
    end
    function varargout = doEstimate(self,varargin)
      [varargout{1:nargout}] = iDynTreeMEX(1626, self, varargin{:});
    end
    function varargout = getLastEstimate(self,varargin)
      [varargout{1:nargout}] = iDynTreeMEX(1627, self, varargin{:});
    end
  end
  methods(Static)
  end
end