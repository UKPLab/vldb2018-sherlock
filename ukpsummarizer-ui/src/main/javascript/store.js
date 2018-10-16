import {createStore, compose, applyMiddleware} from 'redux';
import thunk from 'redux-thunk';
import {routerMiddleware} from 'react-router-redux';

import rootReducer from './reducers';


// if applicable replace this with something that
// reads a previous state from somewhere (i.e. localStorage)
export const loadState = () => {
   if (!localStorage.getItem("users")) {
        return {};
    } else {
        return {
            users: JSON.parse(localStorage.getItem("users"))
        };
    }
};

// if applicable replace this with something that
// writes the state (or parts of it) to somewhere (i.e. localStorage)
export const saveState = (state) => {
    try {
        localStorage.setItem("users", JSON.stringify(state.users));
    } finally {}
};

export const configureStore = (initialState, history) => {
    const store = createStore(
        rootReducer,
        initialState,
        compose(
            applyMiddleware(
                thunk,
                routerMiddleware(history)
            ),
            window.devToolsExtension ? window.devToolsExtension() : f => f
        )
    );

    // replace reducers when hot reloading is active
    if (module.hot) {
        module.hot.accept('reducers', () => (
            store.replaceReducer(require('./reducers'))
        ))
    }

    return store;
};
