import os
import sys
os.environ['CUDA_VISIBLE_DEVICES']=''
import numpy as np
import tensorflow as tf
import load_trace
import a3c
#import fixed_env as env
import get_passes
from get_passes import NUM_OPTIONS
import load_pgm


NUM_PASSES=30
A_DIM = NUM_OPTIONS

S_INFO = NUM_OPTIONS * NUM_PASSES# One-hot encoding for the passes 
S_LEN = 8  # take how many frames in the past

ACTOR_LR_RATE = 0.0001
CRITIC_LR_RATE = 0.001
#VIDEO_BIT_RATE = [300,750,1200,1850,2850,4300]  # Kbps
BUFFER_NORM_FACTOR = 10.0
CHUNK_TIL_VIDEO_END_CAP = 48.0
M_IN_K = 1000.0
REBUF_PENALTY = 4.3  # 1 sec rebuffering -> 3 Mbps
SMOOTH_PENALTY = 1
DEFAULT_QUALITY = 0  # default video quality without agent
RANDOM_SEED = 42
RAND_RANGE = 1000
LOG_FILE = './test_results/log_sim_rl'
TEST_TRACES = './cooked_test_traces/'
# log in format of time_stamp selected_pass buffer_size rebuffer_time chunk_size download_time reward
NN_MODEL = sys.argv[1]


def main():

    np.random.seed(RANDOM_SEED)

    #all_cooked_time, all_cooked_bw, all_file_names = load_trace.load_trace(TEST_TRACES)

    #net_env = env.Environment(all_cooked_time=all_cooked_time,
    #                          all_cooked_bw=all_cooked_bw)

    #log_path = LOG_FILE + '_' + all_file_names[net_env.trace_idx]
    pgm = load_pgm.load_pgm() 
    pgm = pgm[0:2]
    log_path = LOG_FILE
    log_file = open(log_path, 'wb')

    with tf.Session() as sess:

        actor = a3c.ActorNetwork(sess,
                                 state_dim=[S_INFO, S_LEN], action_dim=A_DIM,
                                 learning_rate=ACTOR_LR_RATE)

        critic = a3c.CriticNetwork(sess,
                                   state_dim=[S_INFO, S_LEN],
                                   learning_rate=CRITIC_LR_RATE)

        sess.run(tf.global_variables_initializer())
        saver = tf.train.Saver()  # save neural net parameters

        # restore neural net parameters
        if NN_MODEL is not None:  # NN_MODEL is the path to file
            saver.restore(sess, NN_MODEL)
            print("Testing model restored.")

        time_stamp = 0

        last_selected_pass = DEFAULT_QUALITY
        selected_pass = DEFAULT_QUALITY

        action_vec = np.zeros(A_DIM)
        action_vec[selected_pass] = 1

        s_batch = [np.zeros((S_INFO, S_LEN))]
        a_batch = [action_vec]
        r_batch = []
        entropy_record = []

        pgm_count = 0

        it = 0
        while True:  # serve video forever
            # the action is from the last decision
            # this is to make the framework similar to the real
           # delay, sleep_time, buffer_size, rebuf, \
           # video_chunk_size, next_video_chunk_sizes, \
           # end_of_video, video_chunk_remain = \
           #     net_env.get_video_chunk(selected_pass)

            end_of_opt = ((NUM_PASSES-1) == it)
           # time_stamp += delay  # in ms
           # time_stamp += sleep_time  # in ms

            # reward is video quality - rebuffer penalty - smoothness
           # reward = VIDEO_BIT_RATE[selected_pass] / M_IN_K \
           #          - REBUF_PENALTY * rebuf \
           #          - SMOOTH_PENALTY * np.abs(VIDEO_BIT_RATE[selected_pass] -
           #                                    VIDEO_BIT_RATE[last_selected_pass]) / M_IN_K


            # log time_stamp, selected_pass, buffer_size, reward
            action_index = selected_pass
           # log_file.write(str(time_stamp / M_IN_K) + '\t' +
           #                str(VIDEO_BIT_RATE[selected_pass]) + '\t' +
           #                str(buffer_size) + '\t' +
           #                str(rebuf) + '\t' +
           #                str(video_chunk_size) + '\t' +
           #                str(delay) + '\t' +
           #                str(reward) + '\n')
            # retrieve previous state
            if len(s_batch) == 0:
                state = [np.zeros((S_INFO, S_LEN))]
            else:
                state = np.array(s_batch[-1], copy=True)

            # Apply the optimization  
            linear_idx = it * NUM_OPTIONS + action_index 
            print("Iteration: %d"%it)
            print("action_index: %d"%action_index)
            print("linear_idx: %d"%linear_idx)
  
            # Copy previous state
            state[:, -1] = state[:, -2];
            # Set current state based on action
            state[ linear_idx , -1] = 1;
      
            # Shape of the one-hot encoded passes in this interation is (NUM_OPTIONS, it+1) 
            cur_state = state[:, -1].reshape(-1, NUM_OPTIONS)[0:it+1, :]
            cur_passes = np.argmax(cur_state, axis=1) 
            print("cur_state: ", cur_state)
            print("cur_passes: ", cur_passes)
            reward = get_passes.getTime(pgm[pgm_count], cur_passes)
            log_file.write(
                        str(reward).encode() + b'\n')
 
            log_file.flush()

            # dequeue history record
            state = np.roll(state, -1, axis=1)
            last_selected_pass = selected_pass

            action_prob = actor.predict(np.reshape(state, (1, S_INFO, S_LEN)))
            action_cumsum = np.cumsum(action_prob)
            selected_pass = (action_cumsum > np.random.randint(1, RAND_RANGE) / float(RAND_RANGE)).argmax()
            # Note: we need to discretize the probability into 1/RAND_RANGE steps,
            # because there is an intrinsic discrepancy in passing single state and batch states

            s_batch.append(state)

            entropy_record.append(a3c.compute_entropy(action_prob[0]))

            if end_of_opt:
                log_file.write(b'\n')
                log_file.close()

                last_selected_pass = DEFAULT_QUALITY
                selected_pass = DEFAULT_QUALITY  # use the default action here

                del s_batch[:]
                del a_batch[:]
                del r_batch[:]

                action_vec = np.zeros(A_DIM)
                action_vec[selected_pass] = 1

                s_batch.append(np.zeros((S_INFO, S_LEN)))
                a_batch.append(action_vec)
                entropy_record = []

                pgm_count += 1
                it = 0

                if pgm_count >= len(pgm):
                    break

                #log_path = LOG_FILE + '_' + all_file_names[net_env.trace_idx]
                log_path = LOG_FILE + '_' + str(pgm_count)
                log_file = open(log_path, 'wb')

            it = it + 1

if __name__ == '__main__':
    main()
