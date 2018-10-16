function runtoolboxexamples
% Run all IBM(R) ILOG(R) CPLEX(R) for MATLAB(R) Toolbox examples

% ---------------------------------------------------------------------------
% File: runtoolboxexamples.m
% Version 12.8.0
% ---------------------------------------------------------------------------
% Licensed Materials - Property of IBM
% 5725-A06 5725-A29 5724-Y48 5724-Y49 5724-Y54 5724-Y55 5655-Y21
% Copyright IBM Corporation 2008, 2017. All Rights Reserved.
%
% US Government Users Restricted Rights - Use, duplication or
% disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
% ---------------------------------------------------------------------------

disp ('++cplexlpex')
cplexlpex;
disp ('++cplexbilpex')
cplexbilpex;
disp ('++cplexmilpex')
cplexmilpex;
disp ('++cplexmiqpex')
cplexmiqpex;
disp ('++cplexqcpex')
cplexqcpex;
disp ('++cplexqpex')
cplexqpex;

disp ('++cplexlsqlinex')
cplexlsqlinex;
disp ('++cplexlsqqcpex')
cplexlsqqcpex;
disp ('++cplexlsqbilpex')
cplexlsqbilpex;
disp ('++cplexlsqmilpex')
cplexlsqmilpex;
disp ('++cplexlsqmiqcpex')
cplexlsqmiqcpex;

disp ('++cplexlsqnonnegqcpex')
cplexlsqnonnegqcpex;
disp ('++cplexlsqnonneglinex')
cplexlsqnonneglinex;
disp ('++cplexlsqnonnegmilpex')
cplexlsqnonnegmilpex;
disp ('++cplexlsqnonnegmiqcpex')
cplexlsqnonnegmiqcpex;
end

