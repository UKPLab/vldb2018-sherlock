/**
 * Created by hatieke on 2017-02-06.
 */
import * as Actions from "./constants";

const INITIAL_STATE = {
    loaded: false,
    user: {
        id: null
    }
};

export default function users(prevState = INITIAL_STATE, action) {
    switch (action.type) {
        case Actions.GET_USER_REQUEST:
            return prevState;
        case Actions.GET_USER_SUCCESS:
            return Object.assign({}, prevState, {
                user: action.payload,
                loaded: true
            });
        case Actions.GET_USER_FAIL:
            return prevState;
        case Actions.CREATE_USER_REQUEST:
            return prevState;
        case Actions.CREATE_USER_SUCCESS:
            return prevState;
        case Actions.CREATE_USER_FAIL:
            return prevState;
        default:
            return prevState;
    }
}

