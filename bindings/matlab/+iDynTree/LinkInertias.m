classdef LinkInertias < SwigRef
  methods
    function self = LinkInertias(varargin)
      if nargin~=1 || ~ischar(varargin{1}) || ~strcmp(varargin{1},'_swigCreate')
        % How to get working on C side? Commented out, replaed by hack below
        %self.swigInd = iDynTreeMATLAB_wrap(558, varargin{:});
        tmp = iDynTreeMATLAB_wrap(558, varargin{:}); % FIXME
        self.swigInd = tmp.swigInd;
        tmp.swigInd = uint64(0);
      end
    end
    function varargout = resize(self,varargin)
      [varargout{1:nargout}] = iDynTreeMATLAB_wrap(559, self, varargin{:});
    end
    function varargout = paren(self,varargin)
      [varargout{1:max(1,nargout)}] = iDynTreeMATLAB_wrap(560, self, varargin{:});
    end
    function delete(self)
      if self.swigInd
        iDynTreeMATLAB_wrap(561, self);
        self.swigInd=uint64(0);
      end
    end
  end
  methods(Static)
  end
end