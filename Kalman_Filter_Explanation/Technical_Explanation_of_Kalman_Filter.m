
%------------------------------------------------------------------------
% This is the Matlab version of the EKF() function
% The comments explain how the Kalman Filter operates at each timestep in
% order to generate an SOC prediction closer to the actual.
% 
% Written by Matias Smith, commented by Ayush Saha
%------------------------------------------------------------------------

% The EKF FUNCTION
% Returns xhatCorrected, a matrix with values for [ SOC_estimate_updated,
%                                                        Vc_estimate      ]
% Goal: to accurately estimate SOC value, given our true Vc values and idealied physics laws.
% Note: the EKF Function also returns PCorrected, (which will be explained later).
function [xhatCorrected, PCorrected] = EKF(xhatk_1, Pk_1, I, Ik_1 , V, Voc0, Rk, Aprime, Cprime, Eprime, Fprime, fk, dt, Cbat, Ccap, Rc, Qk1, yk, hk)


    % 1) Calculating estimates for xhat
    % xhat is a matrix with values [ SOC_estimate,
    %		                            Vc_estimate  ]
	% The estimates are based off of idealized physics laws.
    xhat = fk(xhatk_1, I, dt, Cbat, Ccap, Rc);
    
    
    % 2) Updating the ERROR in the estimates, (error_SOC_Estimate stored in P matrix)
    % P is a matrix with values [ error_SOC_Estimate,    value (miscellaneous)
    %                             value (miscellaneous), value (miscellaneous) ]
    % The error_SOC_Estimate is calculated based on the previous error_SOC_Estimate.
    P = Aprime * Pk_1 * Aprime.' + Eprime * Qk1 * Eprime.';


    % 3) KalmanGain = error_SOC_Estimate / (error_SOC_Estimate + error_V_Measurement)
    % NOTE: to find error_Vc_Measurement, we are using:
    %       error_Vc_Measurement = Vc_estimate - Vc_measurement
    % Lk is a matrix with values [ KalmanGain,
    %                              value (miscellaneous) ]
    % The Kalman Gain is closer to 0 if errorMeasurement is relatively large,
    % closer to 1 if errorEstimate is relatively large.
    Lk = P * Cprime.' * (Cprime * P * Cprime.' + Rk)^-1;


    % 4) Updating xhat estimates [ SOC,
    %                               Vc  ]
    % SOC_updated_estimate = SOC_estimate + KalmanGain * (V + Vc - Voc - I*R0)
    % Note:
    %    V: Measured Voltage
    %   Vc: Voltage across capacitor in model battery circuit, calculated
    %       based on I (current) measurement
    %  Voc: Use look up table (specific to vehicle) to determine what the Voc
    %       should be at the current calculated SOC
    % I*R0: I (current) * R0 (resistance)
    %
    % (V + Vc) should theoretically be equal to (Voc - I*R0)
    % This line adjusts xhat to be more accurate
    xhatCorrected = xhat + Lk * (yk(V, xhat(2, 1)) - hk(xhat(1, 1), I, Voc0));
    

    % 5) Setting the predicted values for our next_errorEstimate
    % next_errorEstimate = (1 - KalmanGain) * errorEstimate
    PCorrected = P - Lk * Cprime * P;

end

