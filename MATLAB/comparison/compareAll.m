function comparisonData = compareAll(trials)
    if (nargin < 1 || trials < 1)
        trials = 5;
    end
    
    try
        load('mongoose_data.mat');
    catch fileNotFound
        save('mongoose_data.mat');
        lastMatrixCompleted = 0;
        j = 1;
    end
    
    index = UFget;
    
    for i = (lastMatrixCompleted+1):length(index.nrows)
        if (index.isReal(i))
            
            Prob = UFget(i);
            A = Prob.A;
            
            fprintf('Computing separator for %d: %s\n', i, Prob.name);

            [~, n_cols] = size(A);
            
            % If matrix is unsymmetric, form the augmented system
            if (index.numerical_symmetry(i) < 1)
                [m_rows, n_cols] = size(A);
                A = [sparse(m_rows,m_rows) A; A' sparse(n_cols,n_cols)];
            end
            
            for use_weights = 0:1
                
                % Sanitize the matrix (remove diagonal, take largest scc)
                A = mongoose_sanitizeMatrix(A, ~use_weights);

                % If the sanitization removed all vertices, skip this matrix
                if nnz(A) < 2
                    continue
                end

                % Run Mongoose with various options to partition the graph.
                for guessCutType = 0:2
                    
                    for doCommunityMatching = 0:1
                        
                        for matchingStrategy = 0:3
                            
                            for coarsenLimit = [64, 256, 1024]
                                comparisonData(j).mongoose = 1;
                                comparisonData(j).problem_id = Prob.id;
                                comparisonData(j).problem_name = Prob.name;
                                comparisonData(j).problem_kind = Prob.kind;
                                comparisonData(j).problem_nnz = nnz(A);
                                comparisonData(j).problem_n = n_cols;
                                comparisonData(j).useWeights = use_weights;
                                comparisonData(j).guessCutType = guessCutType;
                                comparisonData(j).doCommunityMatching = doCommunityMatching;
                                comparisonData(j).matchingStrategy = matchingStrategy;
                                comparisonData(j).coarsenLimit = coarsenLimit;
                                
                                fprintf('name = %s\n', Prob.name);
                                fprintf('use_weights = %d\n', use_weights);
                                fprintf('guessCutType = %d\n', guessCutType);
                                fprintf('doCommunityMatching = %d\n', guessCutType);
                                fprintf('matchingStrategy = %d\n9', matchingStrategy);
                                fprintf('coarsenLimit = %d\n', coarsenLimit);
                                
                                for k = 1:trials
                                    % Set up options struct for this run
                                    O = mongoose_getDefaultOptions();
                                    O.randomSeed = 123456789;
                                    O.guessCutType = guessCutType;
                                    O.doCommunityMatching = doCommunityMatching;
                                    O.matchingStrategy = matchingStrategy;
                                    O.coarsenLimit = coarsenLimit;
                                    
                                    tic;
                                    partition = mongoose_computeEdgeSeparator(A,O);
                                    t = toc;
                                    fprintf('Mongoose: %0.2f\n', t);
                                    mongoose_times(j, k) = t;
                                    part_A = find(partition);
                                    part_B = find(1-partition);
                                    perm = [part_A part_B];
                                    p = length(part_A);
                                    A_perm = A(perm, perm);
                                    mongoose_cut_weight(j, k) = sum(sum(A_perm((p+1):n_cols, 1:p)));
                                    mongoose_cut_size(j, k) = sum(sum(sign(A_perm((p+1):n_cols, 1:p))));
                                    mongoose_imbalance(j, k) = abs(0.5-(length(part_A)/(length(part_A) + length(part_B))));
                                end
                                
                                comparisonData(j).time = trimmean(mongoose_times(j, 1:k), 40);
                                comparisonData(j).cutWeight = trimmean(mongoose_cut_weight(j, 1:k), 40);
                                comparisonData(j).cutSize = trimmean(mongoose_cut_size(j, 1:k), 40);
                                comparisonData(j).cutImbalance = trimmean(mongoose_imbalance(j, 1:k), 40);
                                j = j+1;
                            end
                        end
                    end
                end
                
                % Run METIS to partition the graph.
                for k = 1:trials
                    tic;
                    [part_A,part_B] = metispart(A, 0, 123456789);
                    t = toc;
                    fprintf('METIS:    %0.2f\n', t);
                    metis_times(j, k) = t;
                    p = length(part_A);
                    perm = [part_A part_B];
                    A_perm = A(perm, perm);
                    metis_cut_weight(j, k) = sum(sum(A_perm((p+1):n_cols, 1:p)));
                    metis_cut_size(j, k) = sum(sum(sign(A_perm((p+1):n_cols, 1:p))));
                    metis_imbalance(j, k) = abs(0.5-(length(part_A)/(length(part_A) + length(part_B))));
                end
                comparisonData(j).problem_id = Prob.id;
                comparisonData(j).problem_name = Prob.name;
                comparisonData(j).problem_kind = Prob.kind;
                comparisonData(j).problem_nnz = nnz(A);
                comparisonData(j).problem_n = n_cols;
                comparisonData(j).mongoose = 0;
                comparisonData(j).time = trimmean(metis_times(j, 1:k), 40);
                comparisonData(j).cutWeight = trimmean(metis_cut_weight(j, 1:k), 40);
                comparisonData(j).cutSize = trimmean(metis_cut_size(j, 1:k), 40);
                comparisonData(j).cutImbalance = trimmean(metis_imbalance(j, 1:k), 40);
                j = j+1;
            end
        end
        lastMatrixCompleted = i;
        save('mongoose_data.mat');
    end

    % Write data to file for future comparisons
    if(git_found)
        writetable(struct2table(comparisonData), 'mongoose_data.csv');
    end
end