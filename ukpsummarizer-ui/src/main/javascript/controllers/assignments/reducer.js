import * as Actions from "./constants";
import * as AssignmentActions from "../assignment/constants";

const INITIAL_STATE = {
    state: "NEW",
    data: []
};

/**
 * app config is read-only => no actions
 * @param prevState
 * @param action
 * @returns {{title: string}}
 */
export default function assignments(prevState = INITIAL_STATE, action) {
    switch (action.type) {
        case Actions.LOAD_USER_ASSIGNMENTS_RESET: {
            return Object.assign({}, INITIAL_STATE);
        }
        case Actions.LOAD_USER_ASSIGNMENTS_REQUEST:
        case Actions.PATCH_ASSIGNMENT_REQUEST:
            return Object.assign({}, prevState, {
                state: "LOADING",
                data: []
            });
        case Actions.LOAD_USER_ASSIGNMENTS_SUCCESS:
            return Object.assign({}, prevState, {
                state: "LOADED",
                data: action.payload
            });
        case Actions.LOAD_USER_ASSIGNMENTS_FAIL:
        case Actions.PATCH_ASSIGNMENT_FAIL:
            return Object.assign({}, prevState, {
                state: "FAILED",
                data: [],
                error: action.payload
            });
        case AssignmentActions.USER_LOAD_ACTIVE_ASSIGNMENT_SUCCESS:
            const p = action.payload;
            const index = prevState.data.findIndex((e) => e.id === p.id);
            if (index < 0) {
                return prevState;
            }
            let newData = Array.from(prevState.data);
            newData.forEach(e => e.active = false);
            newData[index] = p;
            return Object.assign({}, {
                state: "LOADED",
                data: newData
            });
        default:
            return prevState;
    }

}

