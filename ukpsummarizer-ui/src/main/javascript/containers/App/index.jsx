/**
 * Hendrik Lücke-Tieke
 * dataexpedition.net
 *
 * Copyright (c) 2017 Hendrik Lücke-Tieke. All rights reserved.
 * 
 * Do not use without prior consent by the copyright holder.
 *
 **/

import React from 'react';

import Header from '../../containers/Header';
import Footer from '../../containers/Footer';

import "./app.less";

export const App = ({children}) => (
  <main>
    <Header />
      <div className="appmain">
        {children}
      </div>
    <Footer />
  </main>
);

export default App;

