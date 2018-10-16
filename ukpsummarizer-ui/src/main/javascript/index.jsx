import 'babel-polyfill';

import './styles/global.less';

import React from 'react';
import { render } from 'react-dom';
import { Provider } from 'react-redux';
import { Router, browserHistory } from 'react-router';
import { syncHistoryWithStore } from 'react-router-redux';

import { configureStore, loadState, saveState } from './store';
import createRoutes from './routes';


// create store with persisted state (when available)
const store = configureStore(loadState(), browserHistory);

// sync browser history with store
const history = syncHistoryWithStore(browserHistory, store);

// create routing configuration
const routes = createRoutes(store);

// state persistance
store.subscribe(() => {
  saveState(store.getState())
});

// web analytics callback
history.listen((location) => {
  if (process.env.NODE_ENV === 'production') {
      console.log("Logging interaction");
  } else {
      console.log("Not in 'production' mode");
  }
});

render((
  <Provider store={store}>
    <Router history={browserHistory} routes={routes} />
  </Provider>
), document.getElementById('app-main'));
