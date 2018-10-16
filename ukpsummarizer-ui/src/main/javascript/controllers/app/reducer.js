import * as Actions from "./constants";
import * as Assignments from "../user/constants"

const INITIAL_STATE = {
    title: "Sherlock"
};

/**
 * app config is read-only => no actions
 * @param prevState
 * @param action
 * @returns {{title: string}}
 */
export default function app(prevState = INITIAL_STATE, action) {
    switch (action.type) {

        case Assignments.USER_ACCEPT_ASSIGNMENT_REQUEST:
            return prevState;
        case Assignments.USER_ACCEPT_ASSIGNMENT_SUCCESS:
            return Object.assign({}, prevState, {
                assignment: action.payload
            });
        case Assignments.USER_ACCEPT_ASSIGNMENT_FAIL:
            return prevState;
        default:
            return prevState;
    }

}

