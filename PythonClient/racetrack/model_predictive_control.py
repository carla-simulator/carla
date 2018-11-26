import random
import numpy as np

from scipy.optimize import minimize
from scipy.interpolate import splprep, splev
import sympy as sym
from sympy.tensor.array import derive_by_array
sym.init_printing()

from abstract_controller import Controller

from config import STEER_BOUND, THROTTLE_BOUND


class _EqualityConstraints(object):
    """Class for storing equality constraints in the MPC."""

    def __init__(self, N, state_vars):
        self.dict = {}
        for symbol in state_vars:
            self.dict[symbol] = N*[None]

    def __getitem__(self, key):
        return self.dict[key]

    def __setitem__(self, key, value):
        self.dict[key] = value


class MPCController(Controller):
    def __init__(self, target_speed, steps_ahead=10, dt=0.1):
        self.target_speed = target_speed
        self.state_vars = ('x', 'y', 'v', 'ψ', 'cte', 'eψ')

        self.steps_ahead = steps_ahead
        self.dt = dt

        # Cost function coefficients
        self.cte_coeff = 1000
        self.epsi_coeff = 1000
        self.speed_coeff = 2  # 2
        self.acc_coeff = 10  # 1
        self.steer_coeff = 1  # 1
        self.consec_acc_coeff = 15
        self.consec_steer_coeff = 15

        # Front wheel L
        self.Lf = 2.67  # TODO: check if true

        # How the polynomial fitting the desired curve is fitted
        self.steps_poly = 20
        self.poly_degree = 3

        # Lambdify and minimize stuff
        self.evaluator = 'numpy'
        self.tolerance = 1
        self.cost_func, self.cost_grad_func, self.constr_funcs = self.get_func_constraints_and_bounds()

    def get_func_constraints_and_bounds(self):
        """The most important method of this class, defining the MPC's cost
        function and constraints.
        """
        # Polynomial coefficients will also be symbolic variables
        poly = self.create_array_of_symbols('poly', self.poly_degree+1)

        # Initialize the initial state
        x_init = sym.symbols('x_init')
        y_init = sym.symbols('y_init')
        ψ_init = sym.symbols('ψ_init')
        v_init = sym.symbols('v_init')
        cte_init = sym.symbols('cte_init')
        eψ_init = sym.symbols('eψ_init')

        init = (x_init, y_init, ψ_init, v_init, cte_init, eψ_init)

        # State variables
        x = self.create_array_of_symbols('x', self.steps_ahead)
        y = self.create_array_of_symbols('y', self.steps_ahead)
        ψ = self.create_array_of_symbols('ψ', self.steps_ahead)
        v = self.create_array_of_symbols('v', self.steps_ahead)
        cte = self.create_array_of_symbols('cte', self.steps_ahead)
        eψ = self.create_array_of_symbols('eψ', self.steps_ahead)

        # Actuators
        a = self.create_array_of_symbols('a', self.steps_ahead)
        δ = self.create_array_of_symbols('δ', self.steps_ahead)

        vars_ = (
            # Symbolic arrays (but NOT actuators)
            *x, *y, *ψ, *v, *cte, *eψ,

            # Symbolic arrays (actuators)
            *a, *δ,
        )

        cost = 0
        for t in range(self.steps_ahead):
            cost += (
                # Reference state penalties
                self.cte_coeff * cte[t]**2
                + self.epsi_coeff * eψ[t]**2 +
                + self.speed_coeff * (v[t] - self.target_speed)**2
                #
                # # Actuators penalties
                + self.acc_coeff * a[t]**2
                + self.steer_coeff * δ[t]**2
            )

        # Penalty for differences in consecutive actuators
        for t in range(self.steps_ahead-1):
            cost += (
                self.consec_acc_coeff * (a[t+1] - a[t])**2
                + self.consec_steer_coeff * (δ[t+1] - δ[t])**2
            )

        # Initialize constraints
        eq_constr = _EqualityConstraints(self.steps_ahead, self.state_vars)
        eq_constr['x'][0] = x[0] - x_init
        eq_constr['y'][0] = y[0] - y_init
        eq_constr['ψ'][0] = ψ[0] - ψ_init
        eq_constr['v'][0] = v[0] - v_init
        eq_constr['cte'][0] = cte[0] - cte_init
        eq_constr['eψ'][0] = eψ[0] - eψ_init

        for t in range(1, self.steps_ahead):
            curve = sum(poly[i] * x[t-1]**i for i in range(len(poly)))
            # The desired ψ is equal to the derivative of the polynomial curve at
            #  point x[t-1]
            ψdes = sum(poly[i] * i*x[t-1]**(i-1) for i in range(1, len(poly)))

            eq_constr['x'][t] = x[t] - (x[t-1] + v[t-1] * sym.cos(ψ[t-1]) * self.dt)
            eq_constr['y'][t] = y[t] - (y[t-1] + v[t-1] * sym.sin(ψ[t-1]) * self.dt)
            eq_constr['ψ'][t] = ψ[t] - (ψ[t-1] - v[t-1] * δ[t-1] / self.Lf * self.dt)
            eq_constr['v'][t] = v[t] - (v[t-1] + a[t-1] * self.dt)
            eq_constr['cte'][t] = cte[t] - (curve - y[t-1] + v[t-1] * sym.sin(eψ[t-1]) * self.dt)
            eq_constr['eψ'][t] = eψ[t] - (ψ[t-1] - ψdes - v[t-1] * δ[t-1] / self.Lf * self.dt)

        # Generate actual functions from
        cost_func = self.generate_fun(cost, vars_, init, poly)
        cost_grad_func = self.generate_grad(cost, vars_, init, poly)

        constr_funcs = []
        for symbol in self.state_vars:
            for t in range(self.steps_ahead):
                func = self.generate_fun(eq_constr[symbol][t], vars_, init, poly)
                grad_func = self.generate_grad(eq_constr[symbol][t], vars_, init, poly)
                constr_funcs.append(
                    {'type': 'eq', 'fun': func, 'jac': grad_func, 'args': None},
                )

        return cost_func, cost_grad_func, constr_funcs


    def control(self, pts_2D, measurements, depth_array):
        location = self._extract_location(measurements)
        which_closest, _ = self._find_closest(pts_2D, location)

        indeces = which_closest + self.steps_poly*np.arange(self.poly_degree+1)
        indeces = indeces % pts_2D.shape[0]
        pts = pts_2D[indeces]

        orient = measurements.player_measurements.transform.orientation
        v = measurements.player_measurements.forward_speed * 3.6 # km / h
        ψ = np.arctan2(orient.y, orient.x)

        cos_ψ = np.cos(ψ)
        sin_ψ = np.sin(ψ)

        x, y = location[0], location[1]
        pts_car = MPCController.transform_into_cars_coordinate_system(pts, x, y, cos_ψ, sin_ψ)

        poly = np.polyfit(pts_car[:, 0], pts_car[:, 1], self.poly_degree)

        poly = poly[::-1]
        cte = poly[0]
        eψ = -np.arctan(poly[1])

        init = (0, 0, 0, v, cte, eψ, *poly)
        x0, bounds = self.get_x0_and_bounds(0, 0, 0, v, cte, eψ, 0, 0)
        result = self.minimize_cost(bounds, x0, init)

        # Left here for debugging
        # steer = -0.6 * cte - 5.5 * (cte - prev_cte)
        # prev_cte = cte
        # throttle = clip_throttle(throttle, v, target_speed)

        if 'success' in result.message:
            steer = result.x[-self.steps_ahead]
            throttle = result.x[-2*self.steps_ahead]
        else:
            steer, throttle = None, None

        one_log_dict = {
            'x': x,
            'y': y,
            'orient_x': orient.x,
            'orient_y': orient.y,
            'steer': steer,
            'throttle': throttle,
            'speed': v,
            'psi': ψ,
            'cte': cte,
            'epsi': eψ,
        }
        for i in range(4):
            one_log_dict['poly{}'.format(i)] = poly[i]

        for i in range(pts_car.shape[0]):
            for j in range(pts_car.shape[1]):
                one_log_dict['pts_car_{}_{}'.format(i, j)] = pts_car[i][j]

        return one_log_dict, which_closest

    def get_x0_and_bounds(self, x, y, ψ, v, cte, eψ, a, δ):
        # TODO: impacts performance, you can do better
        x0 = np.array(
            self.steps_ahead * [x]
            + self.steps_ahead * [y]
            + self.steps_ahead * [ψ]
            + self.steps_ahead * [v]
            + self.steps_ahead * [cte]
            + self.steps_ahead * [eψ]
            + self.steps_ahead * [a]
            + self.steps_ahead * [δ]
        )

        # TODO: impacts performance, be better than this
        bounds = (
            6*self.steps_ahead * [(None, None)]
            + self.steps_ahead * [(-THROTTLE_BOUND, THROTTLE_BOUND)]
            + self.steps_ahead * [(-STEER_BOUND, STEER_BOUND)]
        )

        return x0, bounds

    def generate_fun(self, symb_fun, vars_, init, poly):
        '''This function generates a function of the form `fun(x, *args)` because
        that's what the scipy `minimize` API expects (if we don't want to minimize
        over certain variables, we pass them as `args`)
        '''
        args = init + poly
        return sym.lambdify((vars_, *args), symb_fun, self.evaluator)
        # Equivalent to (but faster than):
        # func = sym.lambdify(vars_+init+poly, symb_fun, evaluator)
        # return lambda x, *args: func(*np.r_[x, args])

    def generate_grad(self, symb_fun, vars_, init, poly):
        args = init + poly
        return sym.lambdify(
            (vars_, *args),
            derive_by_array(symb_fun, vars_+args)[:len(vars_)],
            self.evaluator
        )
        # Equivalent to (but faster than):
        # cost_grad_funcs = [
        #     generate_fun(symb_fun.diff(var), vars_, init, poly)
        #     for var in vars_
        # ]
        # return lambda x, *args: [
        #     grad_func(np.r_[x, args]) for grad_func in cost_grad_funcs
        # ]

    def minimize_cost(self, bounds, x0, init):
        # TODO: this is a bit retarded, but hey -- that's scipy API's fault ;)
        for constr_func in self.constr_funcs:
            constr_func['args'] = init

        return minimize(
            fun=self.cost_func,
            x0=x0,
            args=init,
            jac=self.cost_grad_func,
            bounds=bounds,
            constraints=self.constr_funcs,
            method='SLSQP',
            tol=self.tolerance,
        )

    @staticmethod
    def create_array_of_symbols(str_symbol, N):
        return sym.symbols('{symbol}0:{N}'.format(symbol=str_symbol, N=N))

    @staticmethod
    def transform_into_cars_coordinate_system(pts, x, y, cos_ψ, sin_ψ):
        diff = (pts - [x, y])
        pts_car = np.zeros_like(diff)
        pts_car[:, 0] = cos_ψ * diff[:, 0] + sin_ψ * diff[:, 1]
        pts_car[:, 1] = sin_ψ * diff[:, 0] - cos_ψ * diff[:, 1]
        return pts_car
